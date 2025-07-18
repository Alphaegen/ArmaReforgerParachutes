class ParachuteComponentClass : ScriptComponentClass {}
class ParachuteComponent : ScriptComponent {
	protected InputManager m_InputManager;
	protected PlayerController m_PlayerController;

	protected ParachuteItemComponent m_ParachuteItem;

	protected ParachuteDeployedEntity m_DeployedParachute;
	
	SCR_CompartmentAccessComponent m_CompartmentAccess;
	
	[Attribute("12.0", UIWidgets.Slider, "Hard Landing Velocity", "0 50 1", category: "Landing")]
	protected float m_fHardLandingVelocity;
	
	[Attribute("20.0", UIWidgets.Slider, "Death Landing Velocity", "0 100 1", category: "Landing")]
	protected float m_fDeathLandingVelocity;
	
	[Attribute("8.0", UIWidgets.Slider, "Free radius when deploying", "0 100 1", category: "Landing")]
	protected float m_fSafeRadius;
	
	[Attribute("6.0", UIWidgets.Slider, "Minimum fallspeed required", "0 100 1", category: "Landing")]
	protected float m_fMinFallspeed;
	
	[Attribute("10.0", UIWidgets.Slider, "Minimum altitude to deploy", "0 100 1", category: "Landing")]
	protected float m_fMinimumAltitude;
	
	[Attribute("1", UIWidgets.CheckBox, "Break legs", category: "Landing")]
	protected bool m_bBreakLegs;
	
	[Attribute("1", UIWidgets.CheckBox, "Death from speed", category: "Landing")]
	protected bool m_bFallToDeath;

	[RplProp()]
	bool m_bParachuteDeployed = false;
		
	/* state passed into the query callback */
	protected bool m_bNearbyFound;


	private IEntity pilotEntity;
		
	bool HasParachute() { return m_ParachuteItem; }
	
	void SetParachuteItem(ParachuteItemComponent item) {
		m_ParachuteItem = item;
	}
	
	void ClearParachuteItem() {
		m_ParachuteItem = null;
	}
	
	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner) {
		if (SCR_Global.IsEditMode())
			return;

		SCR_PlayerController m_PlayerController = SCR_PlayerController.Cast(owner);
		m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);
		
		m_InputManager = GetGame().GetInputManager();

		EnableComponentControls();
	}
	
	protected void EnableComponentControls() {
		if (!m_InputManager) return;
		
		m_InputManager.AddActionListener("CharacterJump", EActionTrigger.DOWN, OnJumpPressed);
	}

	protected void OnDestroyed(Instigator killer, IEntity killerEntity) {
		DeleteParachuteEntity(m_DeployedParachute);
	}

	void OnControlledEntityChanged(IEntity from, IEntity to) {
		if (!ChimeraCharacter.Cast(to))
			return;

		pilotEntity = to;
		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
		
		DeleteParachuteEntity(m_DeployedParachute);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAskDeployParachute()   // ← new parameter
	{
	    if (m_bParachuteDeployed || !m_ParachuteItem || !pilotEntity || m_ParachuteItem.GetParachuteUsed()) return;
	
	    /* spawn with setup TRUE so components self‑insert */
	    EntitySpawnParams p = new EntitySpawnParams;
	    p.TransformMode = ETransformMode.WORLD;
	    pilotEntity.GetWorldTransform(p.Transform);
		
	    IEntity parachuteEntity = GetGame().SpawnEntityPrefabEx(
	        m_ParachuteItem.GetParachutePrefab(),
	        /*setup*/ false,
	        GetGame().GetWorld(),
	        p
	    );
		m_DeployedParachute = ParachuteDeployedEntity.Cast(parachuteEntity);
		
		giveOwnershipToClient();
		
		vector v0 = pilotEntity.GetPhysics().GetVelocity();
		
		GetGame().GetCallqueue().CallLater(RpcAskSyncReplication, 50, false, m_DeployedParachute.GetRplId());
		GetGame().GetCallqueue().CallLater(RpcAskSyncVelocity, 50, false, v0);
		
		m_bParachuteDeployed = true;
	}
	
	protected void giveOwnershipToClient() {
		PlayerManager playerManager = GetGame().GetPlayerManager();
		SCR_PlayerController playerControler = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerManager.GetPlayerIdFromControlledEntity(pilotEntity)));
	    m_DeployedParachute.GetRplComponent().GiveExt(playerControler.GetRplIdentity(), true);
	}
	
	void RpcAskSyncVelocity(vector v0) {
		Rpc(initializeParachute, v0);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void initializeParachute(vector v0) {
		m_DeployedParachute.InitializePilot(pilotEntity, this, v0);
	    m_ParachuteItem.SetParachuteUsed();
	}
	
	void RpcAskSyncReplication(RplId parachuteRplId) {
		Rpc(RpcDoSyncReplication, parachuteRplId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDoSyncReplication(RplId parachuteRplId) {
		Managed instance = Replication.FindItem(parachuteRplId);
		if (!instance) {
			return;
		}

		RplComponent rplComponent = RplComponent.Cast(instance);
		if (!rplComponent) {
			return;
		}
		
		m_DeployedParachute = ParachuteDeployedEntity.Cast(rplComponent.GetEntity());

		OnChuteReady(m_DeployedParachute);
	}
	
	void OnChuteReady(ParachuteDeployedEntity chute)
	{	
	    m_CompartmentAccess.MoveInVehicle(
	        chute, ECompartmentType.CARGO);

	    chute.EnableControls();
	}
	
	// Break both legs on the server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerBreakLegs()
	{
		if (!m_bBreakLegs) return;
		
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
		if (!m_bFallToDeath) return;
		
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
	
	bool MayDeployParachute()
	{
	    // one chute at a time and you must own one
	    if (m_bParachuteDeployed || !m_ParachuteItem || !HasParachute())
	        return false;
		
		// if parachute has already been used
		if (m_ParachuteItem.GetParachuteUsed())
			return false;
	
	    SCR_ChimeraCharacter pawn = SCR_ChimeraCharacter.Cast(pilotEntity);
	    if (!pawn) return false;
	
	    // no deployment while seated in a vehicle
	    if (pawn.IsInVehicle()) return false;
	
	    // sphere test, early out when another body is close
	    m_bNearbyFound = false;
	    GetGame().GetWorld().QueryEntitiesBySphere(
	        pawn.GetOrigin(),
	        m_fSafeRadius,
	        _CollectFirstNearby,
	        null,
	        EQueryEntitiesFlags.DYNAMIC || EQueryEntitiesFlags.STATIC
	    );
	    if (m_bNearbyFound) return false;
	
	
	    // ordinary free fall, check vertical speed
        float terrainY = SCR_TerrainHelper.GetTerrainY(pawn.GetOrigin(), null, true);
		float heightAGL = pawn.GetOrigin()[1] - terrainY;
	    if (pawn.GetPhysics().GetVelocity()[1] >= -m_fMinFallspeed && heightAGL < m_fMinimumAltitude)
			return false;
		
		return true;
	}
	
	/* callback that stops the query as soon as we see any other actor */
	bool _CollectFirstNearby(IEntity other)
	{
	    // ignore the player himself
	    if (other == pilotEntity) return true;
		
		// ignore items still attached to the pawn such as a backpack
	    if (other.GetParent() == pilotEntity) return false;
		
	    m_bNearbyFound = true;
	    return false;                       // cancel further scanning
	}

	void OnJumpPressed()
	{
	    if (!MayDeployParachute()) return;
		
	    if (Replication.IsServer())
	        RpcAskDeployParachute(); // direct call on dedicated server
	    else
	        Rpc(RpcAskDeployParachute); // remote call to server
	}
	
	/* 1)  NEW SERVER EXIT / DAMAGE RPC  */
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerExitParachute(float velocityAtExit)
	{
	    if (!m_DeployedParachute || !pilotEntity || !m_bParachuteDeployed)
	        return;
		
		/* damage logic – ONLY on server */
        if (velocityAtExit >= m_fHardLandingVelocity && velocityAtExit < m_fDeathLandingVelocity)
            Rpc_ServerBreakLegs();
        else if (velocityAtExit >= m_fDeathLandingVelocity)
            Rpc_ServerKillPlayer();
	    
	    m_CompartmentAccess.AskOwnerToGetOutFromVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.LEAVE_OPEN, true, true);
		
	    GetGame().GetCallqueue().CallLater(DeleteParachuteEntity, 200, false, m_DeployedParachute);
	    m_DeployedParachute = null;
	    m_bParachuteDeployed = false;
	    Replication.BumpMe();
	}
	
	void DeleteParachuteEntity(IEntity parachute)
	{
	    if (parachute)
	        SCR_EntityHelper.DeleteEntityAndChildren(parachute);
			
	}
}
