class ParachuteDeployedEntityClass : GenericEntityClass {}
class ParachuteDeployedEntity : GenericEntity
{
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected RplComponent m_RplComponent;
	protected BaseCompartmentSlot m_Compartment;
	protected Physics m_Physics;
	protected InputManager m_InputManager;
	protected TimeAndWeatherManagerEntity m_weatherManager;
	protected SCR_DamageManagerComponent m_DamageManager;

	protected IEntity m_Pilot;

	// We keep this only to unhook later
	protected SCR_CompartmentAccessComponent m_PilotCompartmentAccess;
	protected bool m_bPilotAccessHooked;

	[Attribute("false", UIWidgets.CheckBox, "Debug Mode", category : "Debug")] protected bool m_DebugMode;

	[Attribute("true", UIWidgets.CheckBox, "Destructable parachute", category : "Etc")] protected bool m_DestructableParachute;

	// === Flight Settings ===
	[Attribute("60", UIWidgets.Slider, "Pitch Torque", "1 200 1", category : "Flight")] protected float m_PitchTorque;

	[Attribute("60", UIWidgets.Slider, "Roll Torque", "1 200 1", category : "Flight")] protected float m_RollTorque;

	[Attribute("5", UIWidgets.Slider, "Drag Strength", "0 100 1", category : "Flight")] protected float m_DragStrength;

	[Attribute("5", UIWidgets.Slider, "Max Fall Speed", "1 20 0.1", category : "Flight")] protected float m_MaxFallSpeed;

	[Attribute("4.0", UIWidgets.Slider, "Glide Acceleration (Pitch Down)", "0 20 0.1", category : "Flight")] protected float m_GlideDownPitch;

	[Attribute("5.0", UIWidgets.Slider, "Glide Deceleration (Pitch Up)", "0 20 0.1", category : "Flight")] protected float m_GlideUpPitch;

	[Attribute("0.0", UIWidgets.Slider, "Min Forward Speed", "0 10 0.1", category : "Flight")] protected float m_MinForwardSpeed;

	[Attribute("21.0", UIWidgets.Slider, "Max Forward Speed", "0 50 0.1", category : "Flight")] protected float m_MaxForwardSpeed;

	[Attribute("200", UIWidgets.Slider, "Auto-Level Proportional gain", "0 500 1", category : "Flight")] protected float m_LevelPropGain;

	[Attribute("20", UIWidgets.Slider, "Auto-Level Damping (Derivative gain)", "0 50 0.1", category : "Flight")] protected float m_LevelDampening;

	[Attribute("2", UIWidgets.Slider, "Auto-Level Power", "0.1 3.0 0.1", category : "Flight")] protected float m_LevelPower;

	[Attribute("45", UIWidgets.Slider, "Turn Max Rate (°/s)", "0 90 1", category : "Flight")] protected float m_MaxTurnRate;

	[Attribute("4.0", UIWidgets.Slider, "Turn Proportional gain", "0 20 0.1", category : "Flight")] protected float m_TurnPropGain;

	[Attribute("1.0", UIWidgets.Slider, "Turn Damping (Derivative gain)", "0 10 0.1", category : "Flight")] protected float m_TurnDampening;

	[Attribute("8", UIWidgets.Slider, "Min Bank Angle to Turn (°)", "0 45 1", category : "Flight")] protected float m_MinBankAngle;

	[Attribute("0.1", UIWidgets.Slider, "Min Pitch Input for Turn", "0 1 0.01", category : "Flight")] protected float m_MinPitchInput;

	// === Landing Settings ===
	[Attribute("20.0", UIWidgets.Slider, "Flare Start Height", "0 100 1", category : "Landing")] protected float m_MaxFlareHeight;

	[Attribute("2.0", UIWidgets.Slider, "Flare End Height", "0 20 1", category : "Landing")] protected float m_MinFlareHeight;

	[Attribute("15.0", UIWidgets.Slider, "Flare Max Deceleration", "0 100 0.5", category : "Landing")] protected float m_MaxFlareDeceleration;

	// === Performance ===
	[Attribute("30.0", UIWidgets.Slider, "Network sync interval (hz)", "1 60 1", category : "Performance")] protected float m_NetworkSyncHz;

	[Attribute("0.10", UIWidgets.Slider, "Net pos threshold (m)", "0 2 0.01", category : "Performance")] protected float m_NetPosThreshM;

	[Attribute("1.0", UIWidgets.Slider, "Net ang threshold (deg)", "0 10 0.1", category : "Performance")] protected float m_NetAngThreshDeg;
	
	// === Performance - Proxy ===
	[Attribute("0.50", UIWidgets.Slider, "Proxy interp factor", "0 1 0.01", category : "Performance")] protected float m_ProxyInterp;

	[Attribute("0.35", UIWidgets.Slider, "Proxy vel interp factor", "0 1 0.01", category : "Performance")] protected float m_ProxyVelInterp;

	[Attribute("0.35", UIWidgets.Slider, "Proxy ang interp factor", "0 1 0.01", category : "Performance")] protected float m_ProxyAngInterp;

	[Attribute("5.0", UIWidgets.Slider, "Proxy max vel delta (m/s)", "0 100 0.5", category : "Performance")] protected float m_ProxyMaxVelDelta;

	[Attribute("90.0", UIWidgets.Slider, "Proxy max ang delta (deg/s)", "0 360 1", category : "Performance")] protected float m_ProxyMaxAngDeltaDeg;
	
	[Attribute("2000", UIWidgets.Slider, "Network cull radius (m)", "0 5000 10", category : "Performance")] protected float m_InterestRadius;

	[Attribute("5.0", UIWidgets.Slider, "Snap distance (m)", "0 50 0.5", category : "Performance")] protected float m_SnapDistanceM;
	
	// === Performance - Owner ===
	[Attribute("1.5", UIWidgets.Slider, "Owner snap distance (m)", "0 10 0.1", category : "Performance")] protected float m_OwnerSnapDistanceM;

	[Attribute("0.35", UIWidgets.Slider, "Owner blend factor", "0 1 0.01", category : "Performance")] protected float m_OwnerBlendFactor;

	[Attribute("10.0", UIWidgets.Slider, "Owner ang snap (deg)", "0 90 1", category : "Performance")] protected float m_OwnerAngSnapDeg;

	[Attribute("0.35", UIWidgets.Slider, "Owner vel blend", "0 1 0.01", category : "Performance")] protected float m_OwnerVelBlend;

	[Attribute("0.35", UIWidgets.Slider, "Owner ang vel blend", "0 1 0.01", category : "Performance")] protected float m_OwnerAngVelBlend;

	[Attribute("30.0", UIWidgets.Slider, "Input send rate (hz)", "1 60 1", category : "Performance")] protected float m_InputSendHz;

	[Attribute("0.01", UIWidgets.Slider, "Input change threshold", "0 0.5 0.01", category : "Performance")] protected float m_InputChangeThreshold;

	[Attribute("0.30", UIWidgets.Slider, "Input timeout (s)", "0 2 0.05", category : "Performance")] protected float m_InputTimeoutSec;

	[Attribute("45.0", UIWidgets.Slider, "Max net speed (m/s)", "0 200 1", category : "Performance")] protected float m_MaxNetSpeed;

	[Attribute("180.0", UIWidgets.Slider, "Max net ang speed (deg/s)", "0 720 5", category : "Performance")] protected float m_MaxNetAngSpeedDeg;

	const float HEADING_LERP_RATE = 2.5;
	const float TWO_PI = 6.283185307;

	protected vector m_vWorldTransform[4];

	protected float m_fInputPitch;
	protected float m_fInputRoll;

	protected vector m_vAngularVelocity;
	protected vector m_vVelocity;

	protected float m_fDownwardVelocity;
	protected float m_fForwardSpeed;

	protected float m_fAccel;
	protected float m_fSmoothAccel;

	protected bool m_bHasLanded;
	protected bool m_bIsDestroyed;

	// Input listener guard
	protected bool m_bControlsEnabled;

	// Perf vars
	protected float m_NetSendInterval;
	protected float m_NetPosThreshSq;
	protected float m_NetAngThreshRad;
	protected float m_ProxyMaxAngDeltaRad;
	protected float m_MaxNetAngSpeedRad;
	protected float m_OwnerAngSnapRad;

	protected float m_InputSendInterval;
	protected float m_InputAccTime;
	protected float m_InputTimeSinceRecv;
	protected float m_LastSentPitch;
	protected float m_LastSentRoll;

	protected float m_NetInputPitch;
	protected float m_NetInputRoll;

	protected float m_WindDirDeg;
	protected float m_WindSpeed;

	protected float m_NetAccTime;
	protected vector m_LastPos;
	protected vector m_LastAnglesRad;

	ref Shape m_DebugWeather;
	ref Shape m_DebugRoll;
	ref Shape m_DebugPitch;
	ref Shape m_DebugBank;
	ref Shape m_DebugFlare;
	ref Shape m_DebugAutoLevel;
	ref Shape m_DebugForwardSpeed;

	bool IsAuthority()
	{
		return m_RplComponent && m_RplComponent.Role() == RplRole.Authority;
	}

	bool IsOwner()
	{
		return m_RplComponent && m_RplComponent.IsOwner();
	}

	RplId GetRplId()
	{
		return m_RplComponent.Id();
	}

	RplComponent GetRplComponent()
	{
		return m_RplComponent;
	}

	void ParachuteDeployedEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	void DebugOverlay()
	{
		if (!m_DebugMode)
			return;

		if (m_Physics)
		{
			m_vVelocity = m_Physics.GetVelocity();
			m_vAngularVelocity = m_Physics.GetAngularVelocity();
			m_fDownwardVelocity = -m_vVelocity[1];
		}

		vector pos = GetOrigin();
		float terrainY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
		float heightAGL = pos[1] - terrainY;
		vector horizVel = m_vVelocity;
		horizVel[1] = 0;
		float forwardSpeedDbg = horizVel.Length();

		DbgUI.Begin("Parachute Debugger");
		DbgUI.Text(string.Format("m_fInputPitch: %1", m_fInputPitch));
		DbgUI.Text(string.Format("m_fInputRoll: %1", m_fInputRoll));
		DbgUI.Text(string.Format("m_vVelocity: %1", m_vVelocity));
		DbgUI.Text(string.Format("m_vAngularVelocity: %1", m_vAngularVelocity));
		DbgUI.Text(string.Format("m_fDownwardVelocity: %1", m_fDownwardVelocity));
		DbgUI.Text(string.Format("m_fForwardSpeed: %1", forwardSpeedDbg));
		DbgUI.Text(string.Format("m_Height: %1", heightAGL));
		DbgUI.Text(string.Format("WindDir: %1", m_WindDirDeg));
		DbgUI.Text(string.Format("WindSpeed: %1", m_WindSpeed));
		DbgUI.End();
	}

	ref Shape DrawDebugLine(vector start, vector end, int color)
	{
		vector lines[2];
		lines[0] = start;
		lines[1] = end;
		return Shape.CreateLines(color, ShapeFlags.DEFAULT, lines, 2);
	}

	void RecalcPerfVars()
	{
		// guard against divide by zero
		float hz = Math.Max(m_NetworkSyncHz, 1.0);
		m_NetSendInterval = 1.0 / hz;
		m_NetPosThreshSq = m_NetPosThreshM * m_NetPosThreshM;
		m_NetAngThreshRad = m_NetAngThreshDeg * Math.DEG2RAD;

		float inputHz = Math.Max(m_InputSendHz, 1.0);
		m_InputSendInterval = 1.0 / inputHz;
		m_ProxyMaxAngDeltaRad = m_ProxyMaxAngDeltaDeg * Math.DEG2RAD;
		m_MaxNetAngSpeedRad = m_MaxNetAngSpeedDeg * Math.DEG2RAD;
		m_OwnerAngSnapRad = m_OwnerAngSnapDeg * Math.DEG2RAD;
	}

	bool ShouldSendSyncState()
	{
		vector pos = GetOrigin();
		if ((pos - m_LastPos).LengthSq() > m_NetPosThreshSq)
			return true;

		vector angDeg = Math3D.MatrixToAngles(m_vWorldTransform);
		vector angRad = angDeg * Math.DEG2RAD;
		if (AngleDeltaRad(angRad[0], m_LastAnglesRad[0]) > m_NetAngThreshRad)
			return true;
		if (AngleDeltaRad(angRad[1], m_LastAnglesRad[1]) > m_NetAngThreshRad)
			return true;
		if (AngleDeltaRad(angRad[2], m_LastAnglesRad[2]) > m_NetAngThreshRad)
			return true;

		return false;
	}

	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		m_InputManager = GetGame().GetInputManager();
		m_Physics = GetPhysics();

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(FindComponent(SCR_BaseCompartmentManagerComponent));
		if (m_CompartmentManager)
			m_Compartment = FindFirstSlotOfType(ECompartmentType.CARGO);

		m_weatherManager = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetTimeAndWeatherManager();

		m_DamageManager = SCR_DamageManagerComponent.Cast(FindComponent(SCR_DamageManagerComponent));
		if (m_DamageManager)
			m_DamageManager.GetOnDamage().Insert(OnParachuteDamaged);

		RecalcPerfVars();

		GetWorldTransform(m_vWorldTransform);
		m_LastPos = m_vWorldTransform[3];
		m_LastAnglesRad = Math3D.MatrixToAngles(m_vWorldTransform) * Math.DEG2RAD;

		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.POSTFRAME | EntityEvent.CONTACT | EntityEvent.SIMULATE);
	}

	BaseCompartmentSlot FindFirstSlotOfType(ECompartmentType type)
	{
		if (!m_CompartmentManager)
			return null;

		array<BaseCompartmentSlot> slots = {};
		m_CompartmentManager.GetCompartments(slots);

		foreach (BaseCompartmentSlot s : slots)
		{
			if (!s)
				continue;
			if (s.GetType() == type)
				return s;
		}

		return null;
	}

	override void EOnDeactivate(IEntity owner)
	{
		DisableControls();
		UnhookPilotAccess();
		UnhookDamage();

		m_Pilot = null;
		m_Physics = null;
		m_InputManager = null;
	}

	// --------------------
	// Frame / simulate
	// --------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (SCR_Global.IsEditMode())
			return;

		if (!m_RplComponent)
			return;

		if (m_bHasLanded)
			return;

		GetWorldTransform(m_vWorldTransform);

		bool isAuthority = IsAuthority();
		bool isOwner = IsOwner();

		if (isAuthority)
		{
			if (m_Compartment && m_Compartment.IsOccupied())
			{
				if (!isOwner)
				{
					if (m_InputTimeSinceRecv > m_InputTimeoutSec)
					{
						m_fInputPitch = 0.0;
						m_fInputRoll = 0.0;
					}
					else
					{
						m_fInputPitch = Math.Clamp(m_NetInputPitch, -1.0, 1.0);
						m_fInputRoll = Math.Clamp(m_NetInputRoll, -1.0, 1.0);
					}
				}
				else
				{
					EnableControls();
				}

				HandlePitch(timeSlice);
				HandleRoll(timeSlice);
				HandleBankTurn(timeSlice);
			}
			else
			{
				m_fInputPitch = 0.0;
				m_fInputRoll = 0.0;
				if (isOwner)
					DisableControls();
			}
		}
		
		if (!isAuthority && isOwner)
		{
			if (m_Compartment && m_Compartment.IsOccupied())
			{
				EnableControls();
				UpdateInputAndSend(timeSlice);
			}
			else
			{
				DisableControls();
				SendZeroInputIfNeeded(timeSlice);
			}

		}
	}
	
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!m_DebugMode)
			return;

		if (!m_RplComponent)
			return;

		if (!IsOwner())
			return;

		DebugOverlay();
	}
	
	void SetPitch(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_bControlsEnabled)
			return;

		m_fInputPitch = value;
	}

	void SetRoll(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_bControlsEnabled)
			return;

		m_fInputRoll = value;
	}

	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!m_RplComponent)
			return;

		if (!IsAuthority() && !IsOwner())
			return;

		if (m_bHasLanded)
			return;

		if (!m_Compartment || !m_Compartment.IsOccupied())
			return;

		AskServerExit();
	}

	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_RplComponent || !m_Physics)
			return;

		if (!IsAuthority())
			return;

		if (m_bHasLanded)
			return;

		if (!IsOwner())
		{
			m_InputTimeSinceRecv += timeSlice;
			if (m_InputTimeSinceRecv > m_InputTimeoutSec)
			{
				m_fInputPitch = 0.0;
				m_fInputRoll = 0.0;
			}
			else
			{
				m_fInputPitch = Math.Clamp(m_NetInputPitch, -1.0, 1.0);
				m_fInputRoll = Math.Clamp(m_NetInputRoll, -1.0, 1.0);
			}
		}

		m_vVelocity = m_Physics.GetVelocity();
		m_vAngularVelocity = m_Physics.GetAngularVelocity();
		m_fDownwardVelocity = -m_vVelocity[1];

		HandleGlide(timeSlice);
		HandleGroundFlare(timeSlice);
		HandleAutoLevel(timeSlice);
		HandleDrag(timeSlice);
		HandleWeather(timeSlice);
		ClampPhysicsState();

		// Ground clip safety
		vector pos = GetOrigin();
		float groundY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
		if (pos[1] - groundY < 0)
			AskServerExit();

		m_NetAccTime += timeSlice;
		if (IsAuthority() && m_NetAccTime >= m_NetSendInterval)
		{
			m_NetAccTime = 0;

			if (ShouldSendSyncState())
			{
				GetWorldTransform(m_vWorldTransform);
				vector vel = m_Physics.GetVelocity();
				vector angVel = m_Physics.GetAngularVelocity();
				SanitizeNetState(m_vWorldTransform, vel, angVel);

				Rpc(RpcDo_SyncMovement, m_vWorldTransform, vel, angVel, m_WindDirDeg, m_WindSpeed);

				m_LastPos = m_vWorldTransform[3];
				m_LastAnglesRad = Math3D.MatrixToAngles(m_vWorldTransform) * Math.DEG2RAD;
			}
		}
	}

	protected void UnhookDamage()
	{
		if (!m_DamageManager)
			return;

		m_DamageManager.GetOnDamage().Remove(OnParachuteDamaged);
		m_DamageManager = null;
	}

	protected void UnhookPilotAccess()
	{
		if (!m_bPilotAccessHooked)
			return;

		if (!m_PilotCompartmentAccess)
			return;

		// If this invoker exists, remove. If it doesn't, we can't do much.
		m_PilotCompartmentAccess.GetOnCompartmentLeft().Remove(OnCompartmentLeft);

		m_bPilotAccessHooked = false;
		m_PilotCompartmentAccess = null;
	}

	void UpdateInputAndSend(float timeSlice)
	{
		if (IsAuthority())
			return;

		if (!IsOwner())
			return;

		m_InputAccTime += timeSlice;
		float pitch = Math.Clamp(m_fInputPitch, -1.0, 1.0);
		float roll = Math.Clamp(m_fInputRoll, -1.0, 1.0);

		bool changed = (Math.AbsFloat(pitch - m_LastSentPitch) > m_InputChangeThreshold) ||
					   (Math.AbsFloat(roll - m_LastSentRoll) > m_InputChangeThreshold);

		if (m_InputAccTime >= m_InputSendInterval && (changed || m_InputAccTime >= (m_InputSendInterval * 3.0)))
		{
			m_InputAccTime = 0;
			m_LastSentPitch = pitch;
			m_LastSentRoll = roll;
			Rpc(RpcAsk_Input, pitch, roll);
		}
	}

	void SendZeroInputIfNeeded(float timeSlice)
	{
		m_fInputPitch = 0.0;
		m_fInputRoll = 0.0;
		UpdateInputAndSend(timeSlice);
	}

	[RplRpc(RplChannel.Unreliable, RplRcver.Server)] void RpcAsk_Input(float pitch, float roll)
	{
		if (!IsAuthority())
			return;

		if (!m_Compartment || !m_Compartment.IsOccupied())
			return;

		m_NetInputPitch = Math.Clamp(pitch, -1.0, 1.0);
		m_NetInputRoll = Math.Clamp(roll, -1.0, 1.0);
		m_InputTimeSinceRecv = 0.0;
	}

	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)] void RpcDo_SyncMovement(vector transform[4], vector vel, vector angVel, float windDirDeg, float windSpeed)
	{
		m_WindDirDeg = windDirDeg;
		m_WindSpeed = windSpeed;
		ApplySyncStateProxy(transform, vel, angVel);
	}

	void ApplySyncStateProxy(vector target[4], vector vel, vector angVel)
	{
		if (IsAuthority())
			return;

		if (IsOwner())
		{
			ApplySyncStateOwner(target, vel, angVel);
			return;
		}

		vector cur[4];
		GetWorldTransform(cur);

		vector posCur = cur[3];
		vector posTgt = target[3];
		float distSq = (posTgt - posCur).LengthSq();

		if (distSq >= (m_SnapDistanceM * m_SnapDistanceM))
		{
			SetWorldTransform(target);
		}
		else
		{
			vector outx[4];
			float t = m_ProxyInterp;

			for (int i = 0; i < 4; i++)
				outx[i] = Lerp(cur[i], target[i], t);

			SetWorldTransform(outx);
		}

		if (m_Physics)
			ApplyProxyVelocitySmoothing(vel, angVel);
	}

	void ApplySyncStateOwner(vector target[4], vector vel, vector angVel)
	{
		vector cur[4];
		GetWorldTransform(cur);

		vector posCur = cur[3];
		vector posTgt = target[3];
		float distSq = (posTgt - posCur).LengthSq();
		float snapSq = m_OwnerSnapDistanceM * m_OwnerSnapDistanceM;

		vector angCurRad = Math3D.MatrixToAngles(cur) * Math.DEG2RAD;
		vector angTgtRad = Math3D.MatrixToAngles(target) * Math.DEG2RAD;
		bool angSnap =
			AngleDeltaRad(angCurRad[0], angTgtRad[0]) > m_OwnerAngSnapRad ||
			AngleDeltaRad(angCurRad[1], angTgtRad[1]) > m_OwnerAngSnapRad ||
			AngleDeltaRad(angCurRad[2], angTgtRad[2]) > m_OwnerAngSnapRad;

		if (distSq >= snapSq || angSnap)
		{
			SetWorldTransform(target);
		}
		else
		{
			vector outx[4];
			float t = m_OwnerBlendFactor;

			for (int i = 0; i < 4; i++)
				outx[i] = Lerp(cur[i], target[i], t);

			SetWorldTransform(outx);
		}

		if (!m_Physics)
			return;

		vector velCur = m_Physics.GetVelocity();
		vector angCur = m_Physics.GetAngularVelocity();

		vector velOut = Lerp(velCur, vel, m_OwnerVelBlend);
		vector angOut = Lerp(angCur, angVel, m_OwnerAngVelBlend);

		m_Physics.SetVelocity(velOut);
		m_Physics.SetAngularVelocity(angOut);
	}

	void ApplyProxyVelocitySmoothing(vector velTarget, vector angVelTarget)
	{
		vector velCur = m_Physics.GetVelocity();
		vector angCur = m_Physics.GetAngularVelocity();

		vector velDelta = velTarget - velCur;
		float velDeltaLen = velDelta.Length();
		if (m_ProxyMaxVelDelta > 0 && velDeltaLen > m_ProxyMaxVelDelta)
			velTarget = velCur + velDelta / velDeltaLen * m_ProxyMaxVelDelta;

		vector angDelta = angVelTarget - angCur;
		float angDeltaLen = angDelta.Length();
		if (m_ProxyMaxAngDeltaRad > 0 && angDeltaLen > m_ProxyMaxAngDeltaRad)
			angVelTarget = angCur + angDelta / angDeltaLen * m_ProxyMaxAngDeltaRad;

		vector velOut = Lerp(velCur, velTarget, m_ProxyVelInterp);
		vector angOut = Lerp(angCur, angVelTarget, m_ProxyAngInterp);

		m_Physics.SetVelocity(velOut);
		m_Physics.SetAngularVelocity(angOut);
	}

	void SanitizeNetState(inout vector transform[4], inout vector vel, inout vector angVel)
	{
		float speed = vel.Length();
		if (speed > m_MaxNetSpeed && speed > 0.001)
			vel = vel / speed * m_MaxNetSpeed;

		float angSpeed = angVel.Length();
		if (angSpeed > m_MaxNetAngSpeedRad && angSpeed > 0.001)
			angVel = angVel / angSpeed * m_MaxNetAngSpeedRad;
	}

	void ClampPhysicsState()
	{
		if (!m_Physics)
			return;

		vector vel = m_Physics.GetVelocity();
		vector angVel = m_Physics.GetAngularVelocity();
		SanitizeNetState(m_vWorldTransform, vel, angVel);
		m_Physics.SetVelocity(vel);
		m_Physics.SetAngularVelocity(angVel);
	}

	vector Lerp(vector a, vector b, float t)
	{
		return a + (b - a) * t;
	}

	float AngleDeltaRad(float a, float b)
	{
		float diff = Math.AbsFloat(a - b);
		if (diff > Math.PI)
			diff = TWO_PI - diff;
		return diff;
	}

	// --------------------
	// Damage / destruction
	// --------------------
	void OnParachuteDamaged(IEntity instigator, float damage, int damageType, int componentID, vector hitWorldPos, vector hitDir)
	{
		if (!IsAuthority())
			return;

		if (m_bIsDestroyed)
			return;

		if (!m_DestructableParachute)
			return;

		if (m_DamageManager && m_DamageManager.GetHealthScaled() <= 0.0)
			DestroyParachute();
	}

	void DestroyParachute()
	{
		if (m_bIsDestroyed)
			return;

		m_bIsDestroyed = true;

		if (m_Compartment && m_Compartment.IsOccupied())
			AskServerExit();

		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 100, false, this);
	}

	// This handles pitching with W and S
	protected void HandlePitch(float timeSlice)
	{
		vector axisWorld = VectorToParent(vector.Right);
		float torque = m_fInputPitch * m_PitchTorque;
		m_Physics.ApplyTorque(axisWorld * torque);

		if (m_DebugMode)
		{
			m_DebugPitch = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2) + (VectorToParent(vector.Forward).Normalized() * (0.05 * torque)), COLOR_BLUE);
		}
	}

	// This handles rolling with A and D
	protected void HandleRoll(float timeSlice)
	{
		vector axisWorld = VectorToParent(vector.Forward);
		float torque = -m_fInputRoll * m_RollTorque;
		m_Physics.ApplyTorque(axisWorld * torque);

		if (m_DebugMode)
		{
			m_DebugRoll = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2) + (VectorToParent(vector.Right).Normalized() * (0.05 * -torque)), COLOR_GREEN);
		}
	}

	void HandleBankTurn(float timeSlice)
	{
		// 1. Read current roll angle (deg) and pitch input (0…1)
		vector ang = Math3D.MatrixToAngles(m_vWorldTransform);
		float rollDeg = ang[2];

		// │bank│ must exceed a small threshold or we don't turn
		if (Math.AbsFloat(rollDeg) < m_MinBankAngle)
			return;

		if (m_fInputPitch < m_MinPitchInput) // must be pulling
			return;

		// 2. Desired yaw rate (deg/s).  Sin gives good feel near ±90° bank.
		float bankFactor = Math.Sin(rollDeg * Math.DEG2RAD); // −1 … 1
		float pitchFactor = Math.Clamp(m_fInputPitch, 0, 1); // 0 … 1
		float yawRateDes = bankFactor * pitchFactor * m_MaxTurnRate;

		// 3. Current yaw rate: project angular velocity on local Y axis
		vector yawAxisWorld = VectorToParent(vector.Up);
		float yawRateCur = vector.Dot(m_vAngularVelocity, yawAxisWorld) * Math.RAD2DEG; // rad/s → deg/s

		// 4. PD torque to drive rate error toward 0
		float rateError = yawRateDes - yawRateCur;
		float torque = rateError * m_TurnPropGain - yawRateCur * m_TurnDampening;

		m_Physics.ApplyTorque(yawAxisWorld * torque);

		if (m_DebugMode)
		{
			m_DebugBank = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1.5), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1.5) + (VectorToParent(vector.Right).Normalized() * (0.2 * torque)), COLOR_RED);
		}
	}

	void HandleGroundFlare(float timeSlice)
	{
		vector pos = GetOrigin();
		float terrainY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
		float height = pos[1] - terrainY;

		// too high, no flare yet
		if (height > m_MaxFlareHeight)
			return;

		// 2. Normalised factor: 0 (start) → 1 (very close)
		float t = Math.Clamp(
			1.0 - (height - m_MinFlareHeight) / (m_MaxFlareHeight - m_MinFlareHeight),
			0.0, 1.0);

		// 3. Extra deceleration
		float extraDecel = t * m_MaxFlareDeceleration;
		float speedLoss = extraDecel * timeSlice;

		m_fForwardSpeed = Math.Max(m_fForwardSpeed - speedLoss, m_MinForwardSpeed);

		// 4. Re-apply new horizontal velocity along current nose direction
		vector forwardW = VectorToParent(vector.Forward);
		vector horizVel = forwardW.Normalized() * m_fForwardSpeed;
		vector newVel = {horizVel[0], m_vVelocity[1], horizVel[2]};

		m_Physics.SetVelocity(newVel);
		m_vVelocity = newVel;

		if (m_DebugMode)
		{
			m_DebugFlare = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1) + (VectorToParent(vector.Forward).Normalized() * t), COLOR_YELLOW);
		}
	}

	void HandleGlide(float timeSlice)
	{
		// 1. World orientation helpers
		vector forwardW = VectorToParent(vector.Forward);
		vector upW = vector.Up;

		// 2. Pitch input
		float pitchDot = vector.Dot(forwardW, upW);
		float normPitch = Math.Sin(Math.Asin(-pitchDot));

		float forwardSpeed = m_fForwardSpeed;
		float verticalSpeed = m_vVelocity[1];

		// Acceleration from pitch
		if (normPitch > 0.03)
			m_fAccel = normPitch * m_GlideDownPitch;
		else if (Math.AbsFloat(normPitch) <= 0.03)
			m_fAccel = m_GlideDownPitch * 0.33;
		else
			m_fAccel = normPitch * m_GlideUpPitch;

		// Low-pass filter
		float smoothing = Math.Clamp(0.5 * timeSlice, 0.0, 0.04);
		m_fSmoothAccel += (m_fAccel - m_fSmoothAccel) * smoothing;

		// Integrate speed
		float prevSpeed = forwardSpeed;
		forwardSpeed = Math.Clamp(
			forwardSpeed + m_fSmoothAccel * timeSlice,
			m_MinForwardSpeed, m_MaxForwardSpeed);

		// Clamp braking per frame
		if (m_fSmoothAccel < 0)
		{
			float maxDrop = 1.5 * timeSlice;
			float actual = prevSpeed - forwardSpeed;
			if (actual > maxDrop)
				forwardSpeed = prevSpeed - maxDrop;
		}

		// Speed-to-lift conversion on hard pull
		if (normPitch < -0.12 && prevSpeed > m_MinForwardSpeed + 1.5)
		{
			float liftFactor = Math.Clamp(-normPitch, 0.0, 1.0);
			float liftConvert = Math.Min(prevSpeed - m_MinForwardSpeed, 2.0) * liftFactor * timeSlice * 0.8;
			verticalSpeed += liftConvert;
			forwardSpeed -= liftConvert * 0.7;
			forwardSpeed = Math.Max(forwardSpeed, m_MinForwardSpeed);
		}

		// 3. Heading blending – keep inertia then steer toward canopy nose
		// Current horizontal direction (ignore Y)
		vector horizVel = m_vVelocity;
		horizVel[1] = 0;
		float horizLen = horizVel.Length();

		vector curDir;
		if (horizLen > 0.01)
			curDir = horizVel / horizLen;
		else
			curDir = forwardW.Normalized();

		// Desired direction is canopy nose projected on horizontal plane
		vector targetDir = forwardW;
		targetDir[1] = 0;
		float tgtLen = targetDir.Length();
		if (tgtLen > 0.001)
			targetDir /= tgtLen;
		else
			targetDir = curDir;

		// Blend current → target by a small fraction each frame
		float lerpAlpha = Math.Clamp(HEADING_LERP_RATE * timeSlice, 0, 1);
		vector newDir = curDir + (targetDir - curDir) * lerpAlpha;
		newDir.Normalize();

		// Compose new horizontal velocity
		vector newHorizVel = newDir * forwardSpeed;

		// 4. Final velocity vector and state updates
		vector newVel = {newHorizVel[0], verticalSpeed, newHorizVel[2]};

		m_Physics.SetVelocity(newVel);
		m_vVelocity = newVel;
		m_fForwardSpeed = forwardSpeed;

		if (m_DebugMode)
		{
			// Debug line that shows the *real* horizontal velocity direction
			m_DebugForwardSpeed = DrawDebugLine(
				GetOrigin(),
				GetOrigin() + (newDir * 3),
				COLOR_RED);
		}
	}

	void HandleAutoLevel(float timeSlice)
	{
		vector worldUp = vector.Up;				// (0 1 0)
		vector actualUp = m_vWorldTransform[1]; // local Y column (canopy up)

		// Error axis and magnitude (sin of tilt angle)
		vector errorAxis = VecCross(actualUp, worldUp);
		float errorMag = errorAxis.Length(); // 0 … 1

		if (errorMag < 0.001)
			return; // already upright
		errorAxis.Normalize();

		// Scale KP with error^m_LevelPower  (e.g. 1.5 => super-linear)
		float kpEff = m_LevelPropGain * Math.Pow(errorMag, m_LevelPower);

		// Angular velocity component along the error axis
		float errorVel = vector.Dot(m_vAngularVelocity, errorAxis);

		vector correctiveTorque =
			errorAxis * (errorMag * kpEff) -		   // P term (scaled)
			errorAxis * (errorVel * m_LevelDampening); // D term

		m_Physics.ApplyTorque(correctiveTorque);

		if (m_DebugMode)
		{
			m_DebugAutoLevel = DrawDebugLine(GetOrigin() - (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() - (VectorToParent(vector.Up).Normalized() * 2) + (actualUp.Normalized() * -correctiveTorque), COLOR_BLUE);
		}
	}

	vector VecCross(vector a, vector b)
	{
		return {
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]};
	}

	void HandleDrag(float timeSlice)
	{
		if (m_fDownwardVelocity <= m_MaxFallSpeed)
			return;

		float excess = m_fDownwardVelocity - m_MaxFallSpeed; // m/s
		float accelNeeded = excess / timeSlice;				 // m/s² that removes the excess in one frame
		accelNeeded = Math.Min(accelNeeded, m_DragStrength); // do not overshoot

		float impulse = accelNeeded * m_Physics.GetMass() * timeSlice; // N·s
		m_Physics.ApplyImpulse(vector.Up * impulse);
	}

	void AskServerExit()
	{
		if (m_bHasLanded)
			return;

		m_bHasLanded = true;
		DisableControls();

		float velocityAtExit = m_fDownwardVelocity;
		Rpc(RpcAsk_ServerExitRequest, velocityAtExit);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)] void RpcAsk_ServerExitRequest(float velocityAtExit)
	{
		if (!m_Compartment)
			return;

		IEntity occupant = m_Compartment.GetOccupant();
		if (!occupant)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		int playerId = pm.GetPlayerIdFromControlledEntity(occupant);

		// API returns 0 when it can't resolve a controlling player.
		if (playerId == 0)
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
		if (!pc)
			return;

		ParachuteComponent parachuteComp = ParachuteComponent.Cast(pc.FindComponent(ParachuteComponent));
		if (!parachuteComp)
			return;

		// Let ParachuteComponent validate chuteId + apply damage + cleanup
		parachuteComp.Rpc_ServerExitParachute(GetRplId(), velocityAtExit);
	}

	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		AskServerExit();
	}

	// --------------------
	// Initialization from owner/controller
	// --------------------
	void InitializePilot(IEntity pilot, SCR_CompartmentAccessComponent access, vector initialVelocity)
	{
		m_Pilot = pilot;

		if (m_Physics)
			m_Physics.SetVelocity(initialVelocity);

		// Optional: listen for pilot leaving compartment so we can trigger exit
		m_PilotCompartmentAccess = access;
		if (m_PilotCompartmentAccess && !m_bPilotAccessHooked)
		{
			m_PilotCompartmentAccess.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			m_bPilotAccessHooked = true;
		}
	}

	void HandleWeather(float timeSlice)
	{
		if (IsAuthority())
		{
			if (!m_weatherManager)
				return;

			m_WindDirDeg = m_weatherManager.GetWindDirection();
			m_WindSpeed = m_weatherManager.GetWindSpeed();
		}
		else
		{
			if (m_WindSpeed == 0)
				return;
		}
		
		vector windDirectionVector;
		windDirectionVector[0] = m_WindDirDeg;

		if (m_DebugMode)
		{
			m_DebugWeather = DrawDebugLine(GetOrigin(), GetOrigin() + windDirectionVector.AnglesToVector(), COLOR_BLUE);
		}

		m_Physics.ApplyImpulse(windDirectionVector.AnglesToVector() * (m_WindSpeed * timeSlice) * 2);
	}

	// --------------------
	// Controls
	// --------------------
	void EnableControls()
	{
		if (!m_InputManager.IsContextActive("CharacterMovementContext"))
			m_InputManager.ActivateContext("CharacterMovementContext");

		if (m_bControlsEnabled)
			return;

		if (!m_InputManager)
			m_InputManager = GetGame().GetInputManager();

		if (!m_InputManager)
			return;

		if (IsAuthority() && !IsOwner())
			return;

		m_bControlsEnabled = true;

		// Character context (on-foot style)
		m_InputManager.AddActionListener("CharacterForward", EActionTrigger.VALUE, SetPitch);
		m_InputManager.AddActionListener("CharacterRight", EActionTrigger.VALUE, SetRoll);

		// Vehicle context fallback (some contexts stop emitting CharacterForward/Right)
		m_InputManager.AddActionListener("VehicleThrottle", EActionTrigger.VALUE, SetPitch);
		m_InputManager.AddActionListener("VehicleSteer", EActionTrigger.VALUE, SetRoll);
	}

	void DisableControls()
	{
		if (!m_bControlsEnabled)
			return;

		m_bControlsEnabled = false;

		if (!m_InputManager)
			return;

		m_InputManager.RemoveActionListener("CharacterForward", EActionTrigger.VALUE, SetPitch);
		m_InputManager.RemoveActionListener("CharacterRight", EActionTrigger.VALUE, SetRoll);

		m_InputManager.RemoveActionListener("VehicleThrottle", EActionTrigger.VALUE, SetPitch);
		m_InputManager.RemoveActionListener("VehicleSteer", EActionTrigger.VALUE, SetRoll);

		// prevent “sticky” input
		m_fInputPitch = 0.0;
		m_fInputRoll = 0.0;
	}
}
