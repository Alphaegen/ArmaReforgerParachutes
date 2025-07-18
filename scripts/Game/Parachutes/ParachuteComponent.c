class ParachuteComponentClass : ScriptComponentClass {}
class ParachuteComponent : ScriptComponent {
	protected InputManager m_InputManager;
	protected RplIdentity m_RplIdentity;
	protected PlayerController m_PlayerController;
	
	protected ParachuteItemComponent m_ParachuteItem;
	protected ParachuteDeployedEntity m_DeployedParachute;
	
	SCR_CompartmentAccessComponent m_CompartmentAccess;
	
	[Attribute("12.0", UIWidgets.Slider, "Hard Landing Velocity", "0 50 1", category: "Landing")]
	protected float m_HardLandingVelocity;
	
	[Attribute("20.0", UIWidgets.Slider, "Death Landing Velocity", "0 100 1", category: "Landing")]
	protected float m_DeathLandingVelocity;
	
	[Attribute("1", UIWidgets.CheckBox, "Break legs", category: "Landing")]
	protected bool m_BreakLegs;
	
	[Attribute("1", UIWidgets.CheckBox, "Death from speed", category: "Landing")]
	protected bool m_FallToDeath;

	[RplProp()]
	protected bool m_ParachuteEquipped = false;

	[RplProp()]
	bool m_ParachuteDeployed = false;

	private IEntity m_target;

	void ~ParachuteComponent() {
		if (m_InputManager) {
			m_InputManager.RemoveActionListener("CharacterJump", EActionTrigger.DOWN, OnCharacterJump);
		}
		
		m_DeployedParachute = null;
	}
		
	bool HasParachute() { return m_ParachuteEquipped; }

	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner) {
		if (SCR_Global.IsEditMode())
			return;

		m_PlayerController = PlayerController.Cast(owner);
		SCR_PlayerController.Cast(owner).m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		SCR_PlayerController.Cast(owner).m_OnDestroyed.Insert(OnDestroyed);

		if (m_PlayerController.GetPlayerId() == SCR_PlayerController.GetLocalPlayerId()) {
			m_InputManager = GetGame().GetInputManager();
			m_InputManager.AddActionListener("CharacterJump", EActionTrigger.DOWN, OnCharacterJump);
		}
	}

	protected void OnDestroyed(Instigator killer, IEntity killerEntity) {
		DeleteParachuteEntity(m_DeployedParachute);
	}

	void OnControlledEntityChanged(IEntity from, IEntity to) {
		if (!ChimeraCharacter.Cast(to))
			return;

		m_target = to;
		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
		
		DeleteParachuteEntity(m_DeployedParachute);
	}
	
	void RegisterItem(ParachuteItemComponent item) {
		if (item) {
			m_ParachuteItem = item;
			m_ParachuteEquipped = true;
		} else {
			m_ParachuteEquipped = false;
			m_ParachuteItem = null;
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcRequestSpawnParachute() {
		if (m_ParachuteDeployed || !m_ParachuteEquipped || !m_target || !m_CompartmentAccess || m_ParachuteItem.GetParachuteUsed())
			return;

		m_ParachuteDeployed = true;

		EntitySpawnParams params = new EntitySpawnParams;
		params.TransformMode = ETransformMode.WORLD;
		m_target.GetWorldTransform(params.Transform);
		params.Transform[3] = params.Transform[3];
		
		IEntity parachuteEntity = GetGame().SpawnEntityPrefabEx(m_ParachuteItem.GetParachutePrefab(), false, GetGame().GetWorld(), params);

		m_DeployedParachute = ParachuteDeployedEntity.Cast(parachuteEntity);
		m_DeployedParachute.PrepareForOccupant(m_target, this);

		PlayerManager playerManager = GetGame().GetPlayerManager();
		SCR_PlayerController pc = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerManager.GetPlayerIdFromControlledEntity(m_target)));

		m_DeployedParachute.GiveExt(pc.GetRplIdentity(), m_CompartmentAccess);
		GetGame().GetCallqueue().CallLater(QueueResponseSpawnParachute, 100, false, m_DeployedParachute.GetRplId());
		
		m_ParachuteItem.SetParachuteUsed();
		
		Replication.BumpMe();
	}

	void QueueResponseSpawnParachute(RplId id) {
		Rpc(RpcResponseSpawnParachute, id);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcResponseSpawnParachute(RplId parachuteRplId) {
		Managed instance = Replication.FindItem(parachuteRplId);
		if (!instance) {
			return;
		}

		RplComponent rplComponent = RplComponent.Cast(instance);
		if (!rplComponent) {
			return;
		}

		m_DeployedParachute = ParachuteDeployedEntity.Cast(rplComponent.GetEntity());

		Physics occupantPhysics = m_target.GetPhysics();
		vector occupantVelocity = occupantPhysics.GetVelocity();
		vector occupantAngVelocity = occupantPhysics.GetAngularVelocity();
		
		Physics deployedParachutePhysics = m_DeployedParachute.GetPhysics();

		deployedParachutePhysics.SetVelocity(occupantVelocity);
		deployedParachutePhysics.SetAngularVelocity(occupantAngVelocity);

		GetGame().GetCallqueue().CallLater(MoveInVehicle, 50, false);
	}
	
	// Break both legs on the server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerBreakLegs()
	{
		if (!m_BreakLegs) return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController) return;
		
		// 1. Validate character and damage component
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(playerController.GetMainEntity());
		if (!ch) return;
	
		SCR_CharacterDamageManagerComponent dmg =
			SCR_CharacterDamageManagerComponent.Cast(ch.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!dmg) return;
		
		int damage = 22;
	
		// 3. Break left and right legs
		array<HitZone> targetHitZones = {};
		dmg.GetHitZonesOfGroup(ECharacterHitZoneGroup.LEFTLEG, targetHitZones, true);
		dmg.GetHitZonesOfGroup(ECharacterHitZoneGroup.RIGHTLEG, targetHitZones, false);
		
		if (targetHitZones.IsEmpty())
			return;
		
		vector hitPosDirNorm[3];
		SCR_DamageContext context = new SCR_DamageContext(EDamageType.COLLISION, damage, hitPosDirNorm, GetOwner(), null, Instigator.CreateInstigator(GetOwner()), null, -1, -1);
		context.damageEffect = new SCR_AnimatedFallDamageEffect();
		
		foreach (HitZone hitZone : targetHitZones)
		{
			context.struckHitZone = hitZone;
			dmg.HandleDamage(context);
		}
	}
	
	// Kill character from the server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerKillPlayer()
	{
		if (!m_FallToDeath) return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController) return;
		
		// 1. Validate character and damage component
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(playerController.GetMainEntity());
		if (!ch) return;
	
		SCR_CharacterDamageManagerComponent dmg =
			SCR_CharacterDamageManagerComponent.Cast(ch.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!dmg) return;
	
		// 3. Death
		dmg.HandleAnimatedFallDamage(200);
	}

	void MoveInVehicle() {
		m_CompartmentAccess.MoveInVehicle(m_DeployedParachute, ECompartmentType.CARGO, true);
		m_DeployedParachute.EnableControls();
	}

	bool CanBeDeployed() {
		if (m_ParachuteDeployed || !m_ParachuteEquipped || !m_target)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_target);
		if (!character || character.IsInVehicle() || !HasParachute())
			return false;

		if (character.GetAnimationComponent().IsRagdollActive()) {
			float height = character.GetOrigin()[1];
			float terrain = SCR_TerrainHelper.GetTerrainY(character.GetOrigin(), null, true);
			return height > terrain + 25;
		}
	
		return m_target.GetPhysics().GetVelocity()[1] <= -2.5;
	}

	void OnCharacterJump(float value) {
		if (!CanBeDeployed())
			return;

		if (Replication.IsServer()) {
			RpcRequestSpawnParachute();
			return;
		}

		Rpc(RpcRequestSpawnParachute);
	}
	
	/* 1)  NEW SERVER EXIT / DAMAGE RPC  */
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerExitParachute(float velocityAtExit)
	{
	    if (!m_DeployedParachute || !m_target || !m_ParachuteDeployed)
	        return;
		
		/* damage logic – ONLY on server */
        if (velocityAtExit >= m_HardLandingVelocity && velocityAtExit < m_DeathLandingVelocity)
            Rpc_ServerBreakLegs();
        else if (velocityAtExit >= m_DeathLandingVelocity)
            Rpc_ServerKillPlayer();
	    
	    m_CompartmentAccess.AskOwnerToGetOutFromVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.LEAVE_OPEN, true, true);
		
	    GetGame().GetCallqueue().CallLater(DeleteParachuteEntity, 300, false, m_DeployedParachute);
	    m_DeployedParachute = null;
	    m_ParachuteDeployed = false;
	    Replication.BumpMe();
	}
	
	void DeleteParachuteEntity(IEntity parachute)
	{
	    if (parachute)
	        SCR_EntityHelper.DeleteEntityAndChildren(parachute);
			
	}

	void SetParachuteEquipped(bool equipped) {
		m_ParachuteEquipped = equipped;
		Replication.BumpMe();
	}
}
