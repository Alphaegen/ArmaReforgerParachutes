class ParachuteComponentClass : ScriptComponentClass {}

class ParachuteComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected InputManager m_InputManager;
	protected SCR_PlayerController m_PlayerController;

	protected IEntity m_PilotEntity;
	protected SCR_CompartmentAccessComponent m_CompartmentAccess;

	// Local convenience pointer (NOT authority)
	protected ParachuteItemComponent m_ParachuteItem;

	// Local resolved entity pointer (NOT replicated)
	protected ParachuteDeployedEntity m_DeployedParachute;

	[Attribute("12.0", UIWidgets.Slider, "Hard Landing Velocity", "0 50 1", category : "Landing")]
	protected float m_fHardLandingVelocity;

	[Attribute("20.0", UIWidgets.Slider, "Death Landing Velocity", "0 100 1", category : "Landing")]
	protected float m_fDeathLandingVelocity;

	[Attribute("8.0", UIWidgets.Slider, "Free radius when deploying", "0 100 1", category : "Landing")]
	protected float m_fSafeRadius;

	[Attribute("10.0", UIWidgets.Slider, "Minimum altitude to deploy", "0 100 1", category : "Landing")]
	protected float m_fMinimumAltitude;

	[Attribute("1", UIWidgets.CheckBox, "Break legs", category : "Landing")]
	protected bool m_bBreakLegs;

	[Attribute("1", UIWidgets.CheckBox, "Death from speed", category : "Landing")]
	protected bool m_bFallToDeath;

	// Replicated deployment state (owner only; others just see the chute entity itself)
	[RplProp(condition: RplCondition.OwnerOnly, onRplName: "OnRep_DeployState")]
	protected bool m_bParachuteDeployed = false;

	[RplProp(condition: RplCondition.OwnerOnly, onRplName: "OnRep_DeployState")]
	protected RplId m_DeployedChuteId = RplId.Invalid();

	// Used by owner client to initialize visuals/controls consistently
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected vector m_vDeployVelocity;
	
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected int m_iChuteSlotId = -1;

	// Query scratch
	protected bool m_bNearbyFound;
	protected IEntity m_QueryPilot;

	bool IsAuthority()
	{
		return m_RplComponent && m_RplComponent.Role() == RplRole.Authority;
	}

	bool IsOwner()
	{
		return m_RplComponent && m_RplComponent.IsOwner();
	}

	bool IsChuteOwner()
	{
		if (!m_DeployedParachute)
			return false;
		
		RplComponent rpl = m_DeployedParachute.GetRplComponent();
		return rpl && rpl.IsOwner();
	}

	bool HasParachute()
	{
		return m_ParachuteItem != null;
	}

	ParachuteItemComponent GetParachuteItem()
	{
		return m_ParachuteItem;
	}

	void SetParachuteItem(ParachuteItemComponent item)
	{
		m_ParachuteItem = item;
	}

	void ClearParachuteItem()
	{
		m_ParachuteItem = null;
	}

	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
			return;

		m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);

		m_InputManager = GetGame().GetInputManager();

		// Prime once if already possessing
		OnControlledEntityChanged(null, m_PlayerController.GetControlledEntity());

		EnableComponentControls();
	}

	override void OnDelete(IEntity owner)
	{
		if (m_PlayerController)
		{
			m_PlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
			m_PlayerController.m_OnDestroyed.Remove(OnDestroyed);
		}

		if (m_InputManager)
			m_InputManager.RemoveActionListener("CharacterJump", EActionTrigger.DOWN, OnJumpPressed);
	}

	protected void EnableComponentControls()
	{
		if (!m_InputManager)
			return;

		if (!m_PlayerController)
			return;

		// Only local owner should listen to input
		int localId = SCR_PlayerController.GetLocalPlayerId();
		if (m_PlayerController.GetPlayerId() != localId)
		    return;


		m_InputManager.AddActionListener("CharacterJump", EActionTrigger.DOWN, OnJumpPressed);
	}

	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{
		// Only authority cleans up world entities
		if (!IsAuthority())
			return;

		DeleteParachuteEntity(m_DeployedParachute);

		m_DeployedParachute = null;
		m_bParachuteDeployed = false;
		m_DeployedChuteId = RplId.Invalid();

		Replication.BumpMe();
	}

	void OnControlledEntityChanged(IEntity from, IEntity to)
	{	
		// Possessing a non-character should clear local pilot references
		SCR_ChimeraCharacter pilot = SCR_ChimeraCharacter.Cast(to);
		if (!pilot)
		{
			m_PilotEntity = null;
			m_CompartmentAccess = null;
			m_ParachuteItem = null;
			return;
		}

		m_PilotEntity = to;
		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));

		// Local convenience for UI checks; server will re-resolve on deploy request
		RefreshParachuteItemFromInventory(to);

		// If the possessed entity changed while a chute exists, authority must clean it up
		if (IsAuthority() && m_bParachuteDeployed)
		{
			DeleteParachuteEntity(m_DeployedParachute);

			m_DeployedParachute = null;
			m_bParachuteDeployed = false;
			m_DeployedChuteId = RplId.Invalid();

			Replication.BumpMe();
		}
	}

	protected void RefreshParachuteItemFromInventory(IEntity pilotEntity)
	{
		m_ParachuteItem = null;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			pilotEntity.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!invMgr)
			return;

		array<IEntity> rootItems = {};
		invMgr.GetItems(rootItems, EStoragePurpose.PURPOSE_ANY);

		foreach (IEntity item : rootItems)
		{
			if (!item)
				continue;

			ParachuteItemComponent pc = ParachuteItemComponent.Cast(item.FindComponent(ParachuteItemComponent));
			if (!pc)
				continue;

			m_ParachuteItem = pc;
			return;
		}
	}

	protected IEntity GetPilotEntity()
	{
		if (m_PilotEntity && SCR_ChimeraCharacter.Cast(m_PilotEntity))
			return m_PilotEntity;

		if (!m_PlayerController)
			return null;

		IEntity ent = m_PlayerController.GetControlledEntity();
		if (!SCR_ChimeraCharacter.Cast(ent))
			return null;

		return ent;
	}

	// --------------------------
	// Deployment checks (shared)
	// --------------------------
	protected bool MayDeployParachute_Internal(IEntity pilot, ParachuteItemComponent item)
	{
		if (!pilot || !item)
			return false;

		if (m_bParachuteDeployed)
			return false;

		if (item.GetParachuteUsed())
			return false;

		SCR_ChimeraCharacter pawn = SCR_ChimeraCharacter.Cast(pilot);
		if (!pawn)
			return false;

		if (pawn.IsInVehicle())
			return false;

		// Safe radius query
		m_QueryPilot = pilot;
		m_bNearbyFound = false;

		GetGame().GetWorld().QueryEntitiesBySphere(
			pawn.GetOrigin(),
			m_fSafeRadius,
			_CollectFirstNearby,
			null,
			EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.STATIC);

		if (m_bNearbyFound)
			return false;

		float terrainY = SCR_TerrainHelper.GetTerrainY(pawn.GetOrigin(), null, true);
		float heightAGL = pawn.GetOrigin()[1] - terrainY;
		if (heightAGL < m_fMinimumAltitude)
			return false;

		return true;
	}

	bool _CollectFirstNearby(IEntity other)
	{
		if (!other)
			return true;

		// Ignore the pilot and anything parented under the pilot (attachments, inventory proxies, etc.)
		if (other == m_QueryPilot)
			return true;

		IEntity p = other.GetParent();
		while (p)
		{
			if (p == m_QueryPilot)
				return true;

			p = p.GetParent();
		}

		// Any other hit cancels scan
		m_bNearbyFound = true;
		return false;
	}

	void OnJumpPressed()
	{
		IEntity pilot = GetPilotEntity();
		if (!pilot)
			return;

		// Client-side pre-check for responsiveness
		if (!m_ParachuteItem)
			return;

		if (!MayDeployParachute_Internal(pilot, m_ParachuteItem))
			return;
		
		if (IsAuthority())
			RpcAskDeployParachute();
		else
			Rpc(RpcAskDeployParachute);
	}

	// --------------------------
	// Server: deploy request
	// --------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAskDeployParachute()
	{
		if (m_bParachuteDeployed)
			return;

		IEntity pilot = GetPilotEntity();
		if (!pilot)
			return;

		// Server MUST re-resolve item from inventory (never trust client pointer)
		ParachuteItemComponent item = ResolveParachuteItem_Server(pilot);
		if (!item)
			return;

		if (!MayDeployParachute_Internal(pilot, item))
			return;

		ResourceName prefab = item.GetParachutePrefab();
		if (prefab == "")
			return;

		EntitySpawnParams sp = new EntitySpawnParams;
		sp.TransformMode = ETransformMode.WORLD;
		pilot.GetWorldTransform(sp.Transform);

				
		
		IEntity spawned = GetGame().SpawnEntityPrefabEx(prefab, false, GetGame().GetWorld(), sp);
		ParachuteDeployedEntity chute = ParachuteDeployedEntity.Cast(spawned);
		if (!chute)
		{
			// If spawn failed, do NOT consume the item
			return;
		}

		// Mark item used on server (replicated!)
		item.SetParachuteUsed_Server();

		m_DeployedParachute = chute;
		m_ParachuteItem = item; // keep local pointer in sync
		
		// --- IMPORTANT: ownership first helps replication race
		GiveChuteOwnershipToController(chute);

		BaseCompartmentManagerComponent bcm = BaseCompartmentManagerComponent.Cast(chute.FindComponent(BaseCompartmentManagerComponent));
		if (!bcm)
			return;
		
		array<BaseCompartmentSlot> slots = {};
		bcm.GetCompartments(slots);
		
		BaseCompartmentSlot pilotSlot = null;
		foreach (BaseCompartmentSlot s : slots)
		{
			if (!s) continue;
			if (s.GetType() == ECompartmentType.CARGO)
			{
				pilotSlot = s;
				break;
			}
		}
		
		if (!pilotSlot)
			return;
		
		m_vDeployVelocity = pilot.GetPhysics().GetVelocity();

		// Initialize on authority so the server sim starts with correct velocity.
		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(
			pilot.FindComponent(SCR_CompartmentAccessComponent));
		chute.InitializePilot(pilot, access, m_vDeployVelocity);
		m_DeployedChuteId = chute.GetRplId();
		m_iChuteSlotId = pilotSlot.GetCompartmentSlotID();
		m_bParachuteDeployed = true;

		Replication.BumpMe();
		GetGame().GetCallqueue().CallLater(Do_SetupDeployedChute_Owner, 50, false, m_DeployedChuteId, m_iChuteSlotId, m_vDeployVelocity);
	}
	
	protected void Do_SetupDeployedChute_Owner(RplId chuteId, int slotId, vector deployVel)
	{
		if (IsChuteOwner())
		{
			RpcDo_SetupDeployedChute_Owner(chuteId, slotId, deployVel);
			return;
		}	
		
		// Rpc(RpcDo_SetupDeployedChute_Owner, chuteId, slotId, deployVel);
	}

	protected ParachuteItemComponent ResolveParachuteItem_Server(IEntity pilotEntity)
	{
		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			pilotEntity.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!invMgr)
			return null;

		array<IEntity> rootItems = {};
		invMgr.GetItems(rootItems, EStoragePurpose.PURPOSE_ANY);

		foreach (IEntity item : rootItems)
		{
			if (!item)
				continue;

			ParachuteItemComponent pc = ParachuteItemComponent.Cast(item.FindComponent(ParachuteItemComponent));
			if (!pc)
				continue;

			return pc;
		}

		return null;
	}

	protected void GiveChuteOwnershipToController(ParachuteDeployedEntity chute)
	{
		if (!chute)
			return;

		RplComponent rpl = chute.GetRplComponent();
		if (!rpl)
			return;

		if (!m_PlayerController)
			return;

		// PlayerController exposes GetRplIdentity() and is commonly used with GiveExt()
		rpl.GiveExt(m_PlayerController.GetRplIdentity(), true);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SetupDeployedChute_Owner(RplId chuteId, int slotId, vector deployVel)
	{
		// Runs where the player controller is local (client, and also listen-host if applicable)
		m_DeployedChuteId = chuteId;
		m_iChuteSlotId = slotId;
		m_vDeployVelocity = deployVel;
		m_bParachuteDeployed = true;
	
		m_DeployedParachute = null;
	
		GetGame().GetCallqueue().CallLater(TryResolveChute_Owner, 50, false);
	}
	
	// --------------------------
	// Owner client: react to replication
	// --------------------------
	void OnRep_DeployState()
	{
		// Only clients care about resolving the spawned entity for control/camera
		if (!GetGame().InPlayMode())
			return;

		if (!m_bParachuteDeployed)
		{
			m_DeployedParachute = null;
			return;
		}

		GetGame().GetCallqueue().CallLater(TryResolveChute_Owner, 50, false);
	}
		
	protected void TryResolveChute_Owner()
	{
		if (!m_bParachuteDeployed)
			return;
	
		if (m_DeployedParachute)
			return;
	
		if (m_DeployedChuteId == RplId.Invalid())
			return;
	
		Managed instance = Replication.FindItem(m_DeployedChuteId);
		RplComponent rplComp = RplComponent.Cast(instance);
		if (!rplComp)
		{
			RetryResolve_Owner();
			return;
		}
	
		ParachuteDeployedEntity chute = ParachuteDeployedEntity.Cast(rplComp.GetEntity());
		if (!chute)
		{
			RetryResolve_Owner();
			return;
		}
	
		m_DeployedParachute = chute;
	
		// Local-only: init visuals (camera/refs) ASAP
		IEntity pilot = GetPilotEntity();
		if (pilot)
			m_DeployedParachute.InitializePilot(pilot, m_CompartmentAccess, m_vDeployVelocity);
	
		// Now actually get in
		TryEnterChute_Owner();
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TryEnterChute_Owner()
	{
		if (!GetGame().InPlayMode())
			return;
	
		if (!m_DeployedParachute)
			return;
	
		if (m_iChuteSlotId < 0)
			return;
	
		IEntity pilot = GetPilotEntity();
		if (!pilot)
			return;
	
		if (!m_CompartmentAccess)
			m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(pilot.FindComponent(SCR_CompartmentAccessComponent));
		if (!m_CompartmentAccess)
			return;
	
		BaseCompartmentManagerComponent bcm = BaseCompartmentManagerComponent.Cast(
			m_DeployedParachute.FindComponent(BaseCompartmentManagerComponent)
		);
		if (!bcm)
		{
			RetryEnterChute_Owner();
			return;
		}
	
		BaseCompartmentSlot slot = bcm.FindCompartment(m_iChuteSlotId);
		if (!slot)
		{
			RetryEnterChute_Owner();
			return;
		}

		if (slot.GetOccupant() == pilot)
		{
			m_iOwnerTries = 0;
			WaitForChuteOwnershipThenEnableControls_Owner();
			return;
		}
	
		bool ok = m_CompartmentAccess.GetInVehicle(
			m_DeployedParachute,                          // slot owner (safe if slot is in child)
			slot,
			true,                                     // forceTeleport
			0,                                        // ignored when forceTeleport==true
			ECloseDoorAfterActions.INVALID,
			true
		);
	
		if (!ok)
		{
			RetryEnterChute_Owner();
			return;
		}
	
		// Enable controls once the chute is actually owned (client ownership can lag 1-2 ticks)
		m_iOwnerTries = 0;
		WaitForChuteOwnershipThenEnableControls_Owner();
	}
	
	protected void RetryEnterChute_Owner()
	{
		GetGame().GetCallqueue().CallLater(TryEnterChute_Owner, 50, false);
	}

	protected void RetryResolve_Owner()
	{
		GetGame().GetCallqueue().CallLater(TryResolveChute_Owner, 50, false);
	}
	
	protected int m_iOwnerTries;

	protected void WaitForChuteOwnershipThenEnableControls_Owner()
	{
		if (!m_DeployedParachute)
			return;
	
		if (IsChuteOwner())
		{
			m_DeployedParachute.EnableControls();
			return;
		}
	
		if (m_iOwnerTries >= 20)
			return;
	
		m_iOwnerTries++;
		GetGame().GetCallqueue().CallLater(WaitForChuteOwnershipThenEnableControls_Owner, 50, false);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_OnParachuteCleared()
	{
		m_bParachuteDeployed = false;
		m_DeployedChuteId = RplId.Invalid();
		m_iChuteSlotId = -1;
		m_DeployedParachute = null;
	}


	// --------------------------
	// Exit / landing (server)
	// --------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerExitParachute(RplId chuteId, float velocityAtExit)
	{
		if (!IsAuthority())
			return;

		if (!m_bParachuteDeployed)
			return;

		if (chuteId != m_DeployedChuteId)
			return;

		// Damage logic only on server
		if (velocityAtExit >= m_fHardLandingVelocity && velocityAtExit < m_fDeathLandingVelocity)
			BreakLegs_Server();
		else if (velocityAtExit >= m_fDeathLandingVelocity)
			KillPlayer_Server();

		// Asks the OWNING client to disembark. This round-trips over the network, so the
		// occupant is NOT guaranteed to have left the compartment this frame (or within
		// any fixed delay) on a loaded MP server.
		if (m_CompartmentAccess)
			m_CompartmentAccess.AskOwnerToGetOutFromVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.LEAVE_OPEN, true, true);

		// Capture refs, then clear deploy state immediately so a second exit request
		// (contact + ground-clip + compartment-left can all fire) can't race this.
		ParachuteDeployedEntity chute = m_DeployedParachute;
		int slotId = m_iChuteSlotId;

		m_DeployedParachute = null;
		m_bParachuteDeployed = false;
		m_DeployedChuteId = RplId.Invalid();
		m_iChuteSlotId = -1;

		Replication.BumpMe();
		Rpc(RpcDo_OnParachuteCleared);

		// Delete ONLY after the compartment is confirmed empty. DeleteEntityAndChildren
		// removes the parachute AND all of its children -- and a still-seated character
		// is a child. If the get-out RPC above has not completed yet (MP latency / peak
		// load), deleting here takes the player with it -> they vanish to the spawn screen.
		m_iExitTries = 0;
		SafeDeleteChuteWhenEmpty(chute, slotId);
	}

	protected int m_iExitTries;

	// Polls the parachute's compartment and deletes only once it is empty, so the
	// occupant is never caught by DeleteEntityAndChildren.
	protected void SafeDeleteChuteWhenEmpty(ParachuteDeployedEntity chute, int slotId)
	{
		if (!chute)
			return;

		bool occupied = false;
		BaseCompartmentManagerComponent bcm = BaseCompartmentManagerComponent.Cast(chute.FindComponent(BaseCompartmentManagerComponent));
		if (bcm)
		{
			BaseCompartmentSlot slot = bcm.FindCompartment(slotId);
			if (slot && slot.IsOccupied())
				occupied = true;
		}

		if (occupied)
		{
			// ~3s cap (60 * 50ms). If the get-out never lands, LEAK the chute rather
			// than risk deleting the player along with it.
			if (m_iExitTries < 60)
			{
				m_iExitTries++;
				GetGame().GetCallqueue().CallLater(SafeDeleteChuteWhenEmpty, 50, false, chute, slotId);
			}
			else
			{
				Print("[Parachute] occupant never disembarked; leaving chute to avoid deleting player", LogLevel.WARNING);
			}
			return;
		}

		DeleteParachuteEntity(chute);
	}

	protected void BreakLegs_Server()
	{
		if (!m_bBreakLegs)
			return;

		if (!m_PlayerController)
			return;

		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
		if (!ch)
			return;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(
			ch.FindComponent(SCR_CharacterDamageManagerComponent));

		if (!dmg)
			return;

		int damage = 22;

		array<HitZone> targetHitZones = {};
		dmg.GetHitZonesOfGroup(ECharacterHitZoneGroup.LEFTLEG, targetHitZones, true);
		dmg.GetHitZonesOfGroup(ECharacterHitZoneGroup.RIGHTLEG, targetHitZones, true);

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

	protected void KillPlayer_Server()
	{
		if (!m_bFallToDeath)
			return;

		if (!m_PlayerController)
			return;

		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
		if (!ch)
			return;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(
			ch.FindComponent(SCR_CharacterDamageManagerComponent));

		if (!dmg)
			return;

		dmg.HandleAnimatedFallDamage(200);
	}

	void DeleteParachuteEntity(IEntity parachute)
	{
		if (parachute)
			SCR_EntityHelper.DeleteEntityAndChildren(parachute);
	}
}
