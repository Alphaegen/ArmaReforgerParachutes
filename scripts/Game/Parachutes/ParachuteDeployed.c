class ParachuteDeployedEntityClass: GenericEntityClass {}
class ParachuteDeployedEntity: GenericEntity {
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected RplComponent m_RplComponent;
	protected BaseCompartmentSlot m_Compartment;
	protected Physics m_Physics;
	protected Animation m_Animation;
	protected InputManager m_InputManager;
	protected TimeAndWeatherManagerEntity m_weatherManager;
	protected SCR_DamageManagerComponent m_DamageManager;
	
	IEntity m_Pilot;

	[Attribute("false", UIWidgets.CheckBox, "Debug Mode", category: "Debug")]
	protected bool m_DebugMode;
	
	[Attribute("true", UIWidgets.CheckBox, "Destructable parachute", category: "Etc")]
	protected bool m_DestructableParachute;
		
	// === Flight Settings ===
	[Attribute("60", UIWidgets.Slider, "Pitch Torque", "1 200 1", category: "Flight")]
	protected float m_PitchTorque;
	
	[Attribute("60", UIWidgets.Slider, "Roll Torque", "1 200 1", category: "Flight")]
	protected float m_RollTorque;
	
	[Attribute("5", UIWidgets.Slider, "Drag Strength", "0 100 1", category: "Flight")]
	protected float m_DragStrength;
	
	[Attribute("5", UIWidgets.Slider, "Max Fall Speed", "1 20 0.1", category: "Flight")]
	protected float m_MaxFallSpeed;
	
	[Attribute("4.0", UIWidgets.Slider, "Glide Acceleration (Pitch Down)", "0 20 0.1", category: "Flight")]
	protected float m_GlideDownPitch;
	
	[Attribute("5.0", UIWidgets.Slider, "Glide Deceleration (Pitch Up)", "0 20 0.1", category: "Flight")]
	protected float m_GlideUpPitch;
	
	[Attribute("0.0", UIWidgets.Slider, "Min Forward Speed", "0 10 0.1", category: "Flight")]
	protected float m_MinForwardSpeed;
	
	[Attribute("21.0", UIWidgets.Slider, "Max Forward Speed", "0 50 0.1", category: "Flight")]
	protected float m_MaxForwardSpeed;
	
	[Attribute("200", UIWidgets.Slider, "Auto-Level Proportional gain", "0 500 1", category: "Flight")]
	protected float m_LevelPropGain;
	
	[Attribute("20", UIWidgets.Slider, "Auto-Level Damping (Derivative gain)", "0 50 0.1", category: "Flight")]
	protected float m_LevelDampening;
	
	[Attribute("2", UIWidgets.Slider, "Auto-Level Power", "0.1 3.0 0.1", category: "Flight")]
	protected float m_LevelPower;
	
	[Attribute("45", UIWidgets.Slider, "Turn Max Rate (°/s)", "0 90 1", category: "Flight")]
	protected float m_MaxTurnRate;
	
	[Attribute("4.0", UIWidgets.Slider, "Turn Proportional gain", "0 20 0.1", category: "Flight")]
	protected float m_TurnPropGain;
	
	[Attribute("1.0", UIWidgets.Slider, "Turn Damping (Derivative gain)", "0 10 0.1", category: "Flight")]
	protected float m_TurnDampening;
	
	[Attribute("8", UIWidgets.Slider, "Min Bank Angle to Turn (°)", "0 45 1", category: "Flight")]
	protected float m_MinBankAngle;
	
	[Attribute("0.1", UIWidgets.Slider, "Min Pitch Input for Turn", "0 1 0.01", category: "Flight")]
	protected float m_MinPitchInput;
	

	// === Landing Settings ===
	[Attribute("20.0", UIWidgets.Slider, "Flare Start Height", "0 100 1", category: "Landing")]
	protected float m_MaxFlareHeight;
	
	[Attribute("2.0", UIWidgets.Slider, "Flare End Height", "0 20 1", category: "Landing")]
	protected float m_MinFlareHeight;
	
	[Attribute("15.0", UIWidgets.Slider, "Flare Max Deceleration", "0 100 0.5", category: "Landing")]
	protected float m_MaxFlareDeceleration;
	
	
	// === Performance ===
	[Attribute("20.0", UIWidgets.Slider, "Network sync interval (hz)", "1 60 1", category: "Performance")]
	protected float m_NetworkSyncHz;
	
	// Pos delta threshold in meters before we consider the state worth a send
	[Attribute("0.10", UIWidgets.Slider, "Net pos threshold (m)", "0 2 0.01", category: "Performance")]
	protected float m_NetPosThreshM;
	
	// Angle delta threshold in degrees
	[Attribute("1.0", UIWidgets.Slider, "Net ang threshold (deg)", "0 10 0.1", category: "Performance")]
	protected float m_NetAngThreshDeg;
	
	// Interp factor applied on proxies for each packet
	[Attribute("0.50", UIWidgets.Slider, "Proxy interp factor", "0 1 0.01", category: "Performance")]
	protected float m_ProxyInterp;
	
	// Optional distance cull. If no remote player is within this range the owner skips sends
	[Attribute("2000", UIWidgets.Slider, "Network cull radius (m)", "0 5000 10", category: "Performance")]
	protected float m_InterestRadius;
	
	// Max error before we hard snap on proxies
	[Attribute("5.0", UIWidgets.Slider, "Snap distance (m)", "0 50 0.5", category: "Performance")]
	protected float m_SnapDistanceM;
	
	const float HEADING_LERP_RATE = 2.5;
	
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
	protected bool m_bIsDestroyed = false;


	ref Shape m_DebugWeather;
	ref Shape m_DebugRoll;
	ref Shape m_DebugPitch;
	ref Shape m_DebugBank;
	ref Shape m_DebugFlare;
	ref Shape m_DebugAutoLevel;
	ref Shape m_DebugForwardSpeed;
	
	RplId GetRplId() { return m_RplComponent.Id(); }
	RplComponent GetRplComponent() { return m_RplComponent; }

	
	void ParachuteDeployedEntity(IEntitySource src, IEntity parent) {
		SetEventMask(EntityEvent.INIT);
	}

	void DebugOverlay() {
		if (!m_DebugMode) return;

		vector pos = GetOrigin();
		float terrainY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
		float heightAGL = pos[1] - terrainY;
		
		DbgUI.Begin("Parachute Debugger");
		DbgUI.Text(string.Format("m_fInputPitch: %1", m_fInputPitch));
		DbgUI.Text(string.Format("m_fInputRoll: %1", m_fInputRoll));
		DbgUI.Text(string.Format("m_vVelocity: %1", m_vVelocity));
		DbgUI.Text(string.Format("m_vAngularVelocity: %1", m_vAngularVelocity));
		DbgUI.Text(string.Format("m_fDownwardVelocity: %1", m_fDownwardVelocity));
		DbgUI.Text(string.Format("m_fForwardSpeed: %1", m_fForwardSpeed));
		DbgUI.Text(string.Format("m_Height: %1", heightAGL));
		DbgUI.End();
	}
	
	ref Shape DrawDebugLine(vector start, vector end, int color) {		
		vector lines[2];
		lines[0] = start;
		lines[1] = end;
		return Shape.CreateLines(color, ShapeFlags.DEFAULT, lines, 2);
	}
	
	void SetPitch(float value)
	{
	    if (!m_RplComponent || !m_RplComponent.IsOwner())
	        return;
	
	    m_fInputPitch = value;
	}
	
	void SetRoll(float value)
	{
	    if (!m_RplComponent || !m_RplComponent.IsOwner())
	        return;
	
	    m_fInputRoll = value;
	}
	
	// Used for performance reasons
	float m_NetSendInterval;    // seconds
	float m_NetPosThreshSq;     // meters squared
	float m_NetAngThreshRad;    // radians
	
	void RecalcPerfVars()
	{
		// guard against divide by zero
		float hz = Math.Max(m_NetworkSyncHz, 1.0);
		m_NetSendInterval = 1.0 / hz;
		m_NetPosThreshSq = m_NetPosThreshM * m_NetPosThreshM;
		m_NetAngThreshRad = m_NetAngThreshDeg * Math.DEG2RAD;
	}

	float         m_NetAccTime;
	vector        m_LastPos;
	vector        m_LastAngles;
	bool ShouldSendSyncState()
	{
		// 1. Interest check
		if (!IsAnyRemotePlayerWithin(m_InterestRadius))
			return false;
	
		// 2. Position delta
		vector pos = GetOrigin();    // or m_vWorldTransform[3] if already fetched
		if ((pos - m_LastPos).LengthSq() > m_NetPosThreshSq)
			return true;
	
		// 3. Angle delta
		vector ang = Math3D.MatrixToAngles(m_vWorldTransform);
		if (Math.AbsFloat(ang[0] - m_LastAngles[0]) > m_NetAngThreshDeg ||
		    Math.AbsFloat(ang[1] - m_LastAngles[1]) > m_NetAngThreshDeg ||
		    Math.AbsFloat(ang[2] - m_LastAngles[2]) > m_NetAngThreshDeg)
			return true;
	
		return false;
	}
	
	bool IsAnyRemotePlayerWithin(float radius)
	{
		// On a client we cannot inspect other proxies, always return true
		if (!Replication.IsServer())
			return true;
	
		PlayerManager pm = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		pm.GetPlayers(playerIds);               // fills the array with IDs
	
		if (playerIds.Count() <= 1)             // only the local player in session
			return false;
	
		vector myPos = GetOrigin();
		float  radiusSq = radius * radius;
	
		foreach (int pid : playerIds)
		{
			IEntity ent = pm.GetPlayerControlledEntity(pid);
			if (!ent)
				continue;
	
			// Skip the occupant of this parachute
			if (m_Compartment && ent == m_Compartment.GetOccupant())
				continue;
	
			// Horizontal distance only
			vector delta = { ent.GetOrigin()[0] - myPos[0], 0, ent.GetOrigin()[2] - myPos[2] };
			if (delta.LengthSq() <= radiusSq)
				return true;
		}
		return false;
	}
	
	override void EOnInit(IEntity owner) {
		if (SCR_Global.IsEditMode())
			return;

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		m_InputManager = GetGame().GetInputManager();

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(FindComponent(SCR_BaseCompartmentManagerComponent));
		m_Compartment = m_CompartmentManager.FindCompartment(2);

		m_Physics = GetPhysics();

		m_weatherManager = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetTimeAndWeatherManager();
		
		m_DamageManager = SCR_DamageManagerComponent.Cast(FindComponent(SCR_DamageManagerComponent));
	    if (m_DamageManager)
	    {
	        m_DamageManager.GetOnDamage().Insert(OnParachuteDamaged);
	    }

		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.CONTACT | EntityEvent.SIMULATE);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice) {			
		if (!m_RplComponent.IsOwner())
	        return;
		
		GetWorldTransform(m_vWorldTransform);
		
		if (m_Compartment.IsOccupied()) {
			if (!m_InputManager.IsContextActive("CharacterMovementContext"))
			    m_InputManager.ActivateContext("CharacterMovementContext");						
			HandlePitch(timeSlice);
			HandleRoll(timeSlice);
			HandleBankTurn(timeSlice);
			HandleWeather(timeSlice);
		}

		if (m_DebugMode) {
			DebugOverlay();
		}
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
	    if (!m_RplComponent.IsOwner())
	        return;
	
	    if (m_bHasLanded || !m_Compartment.IsOccupied())
	        return;
	

	    AskServerExit();
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
	    if (!m_RplComponent.IsOwner())
			return;

		m_vVelocity = m_Physics.GetVelocity();
		m_vAngularVelocity = m_Physics.GetAngularVelocity();
		m_fDownwardVelocity = -m_vVelocity[1];

	    HandleGlide(timeSlice);
	    HandleGroundFlare(timeSlice);
	    HandleAutoLevel(timeSlice);
	    HandleDrag(timeSlice);

	    vector pos = GetOrigin();
	    float groundY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
	    if (pos[1] - groundY < 0)
	    {
	        AskServerExit();
	    }

	   m_NetAccTime += timeSlice;
		if (m_NetAccTime >= m_NetSendInterval)
		{
			m_NetAccTime = 0;
			if (ShouldSendSyncState())     // new gate
			{
				GetWorldTransform(m_vWorldTransform);
				Rpc(RpcAsk_SyncMovement, m_vWorldTransform, m_Physics.GetVelocity(), m_Physics.GetAngularVelocity());
				// record last sent for delta gating
				m_LastPos = m_vWorldTransform[3];
				m_LastAngles = Math3D.MatrixToAngles(m_vWorldTransform);
			}
		}
	}
	
	/* Called by the owner (client or server) every 33 ms */
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void RpcAsk_SyncMovement(vector transform[4], vector vel, vector angVel)
	{
		// first update authoritative copy on the server
		ApplySyncState(transform, vel, angVel);
	
		// now forward to every proxy except the owner
		Rpc(RpcDo_SyncMovement, transform, vel, angVel);
	}
	
	void ApplySyncState(vector transform[4], vector vel, vector angVel)
	{
		if (m_RplComponent.IsOwner()) return;   // never overwrite the local owner
		SetWorldTransform(transform);
		m_Physics.SetVelocity(vel);
		m_Physics.SetAngularVelocity(angVel);
	}
	
	/* Executed on proxies that are NOT the owner */
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast, RplCondition.NoOwner)]
	void RpcDo_SyncMovement(vector transform[4], vector vel, vector angVel)
	{
		ApplySyncStateProxy(transform, vel, angVel);
	}
	
	void ApplySyncStateProxy(vector target[4], vector vel, vector angVel)
	{
		if (m_RplComponent.IsOwner())
			return;
	
		// Current transform
		vector cur[4];
		GetWorldTransform(cur);
	
		// Pos error
		vector posCur = cur[3];
		vector posTgt = target[3];
		float distSq = (posTgt - posCur).LengthSq();
	
		if (distSq >= (m_SnapDistanceM * m_SnapDistanceM))
		{
			// big error. snap hard
			SetWorldTransform(target);
		}
		else
		{
			// smooth lerp of each column
			vector outx[4];
			float t = m_ProxyInterp;
			for (int i = 0; i < 4; i++)
				outx[i] = Lerp(cur[i], target[i], t);
			SetWorldTransform(outx);
		}
	
		// Velocity
		m_Physics.SetVelocity(vel);
		m_Physics.SetAngularVelocity(angVel);
	}
	
	vector Lerp(vector a, vector b, float t)
	{
	    return a + (b - a) * t;
	}
	
	void OnParachuteDamaged(IEntity instigator, float damage, int damageType, int componentID, vector hitWorldPos, vector hitDir)
	{
	    if (!Replication.IsServer() || m_bIsDestroyed || !m_DestructableParachute)
	        return;
	
	    if (m_DamageManager && m_DamageManager.GetHealthScaled() <= 0.0)
	    {
	        DestroyParachute();
	    }
	}
	
	void DestroyParachute()
	{
	    if (m_bIsDestroyed)
	        return;
	
	    m_bIsDestroyed = true;
	
	    // 1. Kick occupant (if any) immediately
	    if (m_Compartment && m_Compartment.IsOccupied())
	    {
	        AskServerExit();        // uses your existing exit RPC
	    }
	
	    // 2. Delete the entity after the occupant has been moved out
	    GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 100, false, this);
	}

		
	// This handles pitching with W and S
	protected void HandlePitch(float timeSlice) {
		vector axisWorld = VectorToParent(vector.Right);
	    float torque     = m_fInputPitch * m_PitchTorque;
	    m_Physics.ApplyTorque(axisWorld * torque);
		
		if (m_DebugMode) {
			m_DebugPitch = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2) + (VectorToParent(vector.Forward).Normalized() * (0.05 * torque)), COLOR_BLUE);
		}
	}
	
	// This handles rolling with A and D
	protected void HandleRoll(float timeSlice) {
		vector axisWorld = VectorToParent(vector.Forward);
	    float torque     = -m_fInputRoll * m_RollTorque;
	    m_Physics.ApplyTorque(axisWorld * torque);
		
		if (m_DebugMode) {
			m_DebugRoll = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 2) + (VectorToParent(vector.Right).Normalized() * (0.05 * -torque)), COLOR_GREEN);
		}

	}
	
	void HandleBankTurn(float timeSlice) {
		// 1. Read current roll angle (deg) and pitch input (0…1)
	    vector ang     = Math3D.MatrixToAngles(m_vWorldTransform);
	    float  rollDeg = ang[2];
	
	    // │bank│ must exceed a small threshold or we don't turn
	    if (Math.AbsFloat(rollDeg) < m_MinBankAngle)
	        return;
	
	    if (m_fInputPitch < m_MinPitchInput)      // must be pulling
	        return;
	
	    // 2. Desired yaw rate (deg/s).  Sin gives good feel near ±90° bank.
	    float bankFactor   = Math.Sin(rollDeg * Math.DEG2RAD);      // −1 … 1
	    float pitchFactor  = Math.Clamp(m_fInputPitch, 0, 1);       // 0 … 1
	    float yawRateDes   = bankFactor * pitchFactor * m_MaxTurnRate;
	
	    // 3. Current yaw rate: project angular velocity on local Y axis
	    vector yawAxisWorld = VectorToParent(vector.Up);
	    float   yawRateCur  = vector.Dot(m_vAngularVelocity, yawAxisWorld) * Math.RAD2DEG; // rad/s → deg/s
	
	    // 4. PD torque to drive rate error toward 0
	    float rateError = yawRateDes - yawRateCur;
	    float torque    = rateError * m_TurnPropGain - yawRateCur * m_TurnDampening;
	
	    m_Physics.ApplyTorque(yawAxisWorld * torque);
		
		if (m_DebugMode) {
			m_DebugBank = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1.5), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1.5) + (VectorToParent(vector.Right).Normalized() * (0.2 * torque)), COLOR_RED);
		}
	}
	
	void HandleGroundFlare(float timeSlice) {
		vector pos       = GetOrigin();
		float   terrainY = SCR_TerrainHelper.GetTerrainY(pos, null, true);
		float   height   = pos[1] - terrainY;
	
		// too high, no flare yet
		if (height > m_MaxFlareHeight)
			return;  
	
		// 2. Normalised factor: 0 (start) → 1 (very close)
		float t = Math.Clamp(
			1.0 - (height - m_MinFlareHeight) / (m_MaxFlareHeight - m_MinFlareHeight),
			0.0, 1.0);
	
		// 3. Extra deceleration
		float extraDecel = t * m_MaxFlareDeceleration;
		float speedLoss  = extraDecel * timeSlice;
	
		m_fForwardSpeed = Math.Max(m_fForwardSpeed - speedLoss, m_MinForwardSpeed);
	
		// 4. Re-apply new horizontal velocity along current nose direction
		vector forwardW = VectorToParent(vector.Forward);
		vector horizVel = forwardW.Normalized() * m_fForwardSpeed;
		vector newVel   = { horizVel[0], m_vVelocity[1], horizVel[2] };
	
		m_Physics.SetVelocity(newVel);
		m_vVelocity = newVel;
		
		if (m_DebugMode) {
			m_DebugFlare = DrawDebugLine(GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1), GetOrigin() + (VectorToParent(vector.Up).Normalized() * 1) + (VectorToParent(vector.Forward).Normalized() * t), COLOR_YELLOW);
		}
	}
	
	void HandleGlide(float timeSlice) {
	    // 1. World orientation helpers
	    vector forwardW = VectorToParent(vector.Forward);
	    vector upW      = vector.Up;
	
	    // 2. Pitch input
	    float pitchDot  = vector.Dot(forwardW, upW);
	    float normPitch = Math.Sin(Math.Asin(-pitchDot));
	
	    float forwardSpeed  = m_fForwardSpeed;
	    float verticalSpeed = m_vVelocity[1];
	
	    // Acceleration from pitch
	    if (normPitch > 0.03)
	        m_fAccel =  normPitch * m_GlideDownPitch;
	    else if (Math.AbsFloat(normPitch) <= 0.03)
	        m_fAccel =  m_GlideDownPitch * 0.33;
	    else
	        m_fAccel =  normPitch * m_GlideUpPitch;
		
	    // Low-pass filter
	    float smoothing     = Math.Clamp(0.5 * timeSlice, 0.0, 0.04);
	    m_fSmoothAccel      += (m_fAccel - m_fSmoothAccel) * smoothing;
		
	    // Integrate speed
	    float prevSpeed     = forwardSpeed;
	    forwardSpeed        = Math.Clamp(
	                            forwardSpeed + m_fSmoothAccel * timeSlice,
	                            m_MinForwardSpeed, m_MaxForwardSpeed);
		
	    // Clamp braking per frame
	    if (m_fSmoothAccel < 0)
	    {
	        float maxDrop   = 1.5 * timeSlice;
	        float actual    = prevSpeed - forwardSpeed;
	        if (actual > maxDrop)
	            forwardSpeed = prevSpeed - maxDrop;
	    }
	
	    // Speed-to-lift conversion on hard pull
	    if (normPitch < -0.12 && prevSpeed > m_MinForwardSpeed + 1.5)
	    {
	        float liftFactor  = Math.Clamp(-normPitch, 0.0, 1.0);
	        float liftConvert = Math.Min(prevSpeed - m_MinForwardSpeed, 2.0)
	                            * liftFactor * timeSlice * 0.8;
	        verticalSpeed    += liftConvert;
	        forwardSpeed     -= liftConvert * 0.7;
	        forwardSpeed      = Math.Max(forwardSpeed, m_MinForwardSpeed);
	    }
	
	    // 3. Heading blending – keep inertia then steer toward canopy nose
	    // Current horizontal direction (ignore Y)
	    vector horizVel     = m_vVelocity;
	    horizVel[1]         = 0;
	    float  horizLen     = horizVel.Length();
	
	    vector curDir;
	    if (horizLen > 0.01)     curDir = horizVel / horizLen;
	    else                     curDir = forwardW.Normalized();
	
	    // Desired direction is canopy nose projected on horizontal plane
	    vector targetDir     = forwardW;
	    targetDir[1]         = 0;
	    float  tgtLen        = targetDir.Length();
	    if (tgtLen > 0.001)   targetDir /= tgtLen;
	    else                  targetDir  = curDir;
	
	    // Blend current → target by a small fraction each frame
	    float  lerpAlpha     = Math.Clamp(HEADING_LERP_RATE * timeSlice, 0, 1);
	    vector newDir        = curDir + (targetDir - curDir) * lerpAlpha;
	    newDir.Normalize();
	
	    // Compose new horizontal velocity
	    vector newHorizVel   = newDir * forwardSpeed;
	
	    // 4. Final velocity vector and state updates
	    vector newVel        = { newHorizVel[0], verticalSpeed, newHorizVel[2] };
	
	    m_Physics.SetVelocity(newVel);
	    m_vVelocity   = newVel;
	    m_fForwardSpeed = forwardSpeed;
			
		if (m_DebugMode) {
		    // Debug line that shows the *real* horizontal velocity direction
		    m_DebugForwardSpeed = DrawDebugLine(
		        GetOrigin(),
		        GetOrigin() + (newDir * 3),
		        COLOR_RED);
		}
	}
	
	void HandleAutoLevel(float timeSlice) {
		vector worldUp  = vector.Up;            // (0 1 0)
		vector actualUp = m_vWorldTransform[1];  // local Y column (canopy up)
	
		// Error axis and magnitude (sin of tilt angle)
		vector errorAxis = VecCross(actualUp, worldUp);
		float  errorMag  = errorAxis.Length();          // 0 … 1
	
		if (errorMag < 0.001) return;                   // already upright
		errorAxis.Normalize();
	
		// Scale KP with error^m_LevelPower  (e.g. 1.5 ⇒ super-linear)
		float kpEff = m_LevelPropGain * Math.Pow(errorMag, m_LevelPower);
	
		// Angular velocity component along the error axis
		float errorVel = vector.Dot(m_vAngularVelocity, errorAxis);
	
		vector correctiveTorque =
			errorAxis * (errorMag * kpEff) -         // P term (scaled)
			errorAxis * (errorVel * m_LevelDampening);       // D term
	
		m_Physics.ApplyTorque(correctiveTorque);
		
		if (m_DebugMode) {
			m_DebugAutoLevel = DrawDebugLine(GetOrigin() - (VectorToParent(vector.Up).Normalized() * 2), GetOrigin() - (VectorToParent(vector.Up).Normalized() * 2) + (actualUp.Normalized() * -correctiveTorque), COLOR_BLUE);
		}
	}
	
	vector VecCross(vector a, vector b)
	{
		return {
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]
		};
	}
	
	void HandleDrag(float timeSlice)
	{
	    if (m_fDownwardVelocity <= m_MaxFallSpeed)
	        return;
	
	    float excess      = m_fDownwardVelocity - m_MaxFallSpeed;   // m/s
	    float accelNeeded = excess / timeSlice;    // m/s² that removes the excess in **one** frame
	    accelNeeded       = Math.Min(accelNeeded, m_DragStrength);  // do not overshoot
	
	    float impulse     = accelNeeded * m_Physics.GetMass() * timeSlice; // N·s
	    m_Physics.ApplyImpulse(vector.Up * impulse);
	}

	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
	    AskServerExit();
	}
	
	void AskServerExit()
	{
		if (m_bHasLanded) return;
		
	    float velocityAtExit = m_fDownwardVelocity;
	    Rpc(RpcAsk_ServerExitRequest, velocityAtExit);
		
		m_bHasLanded = true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ServerExitRequest(float velocityAtExit)
	{
	 /* 1.  Who is sitting in the parachute? */
	    IEntity occupant = m_Compartment.GetOccupant();
	    if (!occupant)
	        return;
	
	    /* 2.  Resolve the PlayerController that owns this pawn */
	    PlayerManager pm = GetGame().GetPlayerManager();
	    int playerId = pm.GetPlayerIdFromControlledEntity(occupant);
	    if (playerId == 0)
	        return; // not under control right now
	
	    PlayerController pc = pm.GetPlayerController(playerId);
	    if (!pc)
	        return;
	
	    /* 3.  Grab the ParachuteComponent from the controller itself */
	    ParachuteComponent parachuteComp = ParachuteComponent.Cast(
	        pc.FindComponent(ParachuteComponent));
	    if (!parachuteComp)
	        return;
	
	    /* 4.  Ask it to do the authoritative exit */
	    parachuteComp.Rpc_ServerExitParachute(velocityAtExit);
	}
	
	void InitializePilot(IEntity pilot, ParachuteComponent parachuteComponent, vector velocity) {
		m_Pilot = pilot;
		parachuteComponent.m_CompartmentAccess.GetOnCompartmentLeft().Insert(OnCompartmentLeft);		
		m_Physics.SetVelocity(velocity);
	}

	void HandleWeather(float timeSlice) {
		if (!m_weatherManager)
			return;
		
		vector windDirectionVector;
		windDirectionVector[0] = m_weatherManager.GetWindDirection();
		
		if (m_DebugMode) {
			m_DebugWeather = DrawDebugLine(GetOrigin(), GetOrigin() + windDirectionVector.AnglesToVector(), COLOR_BLUE);
		}
		
		m_Physics.ApplyImpulse(windDirectionVector.AnglesToVector() * (m_weatherManager.GetWindSpeed() * timeSlice) * 2);
	}
	
	void EnableControls() {
		m_InputManager.AddActionListener("CharacterForward", EActionTrigger.VALUE, SetPitch);
		m_InputManager.AddActionListener("CharacterRight", EActionTrigger.VALUE, SetRoll);	
	}
	
	void DisableControls() {
		m_InputManager.RemoveActionListener("CharacterForward", EActionTrigger.VALUE, SetPitch);
		m_InputManager.RemoveActionListener("CharacterRight", EActionTrigger.VALUE, SetRoll);
	}

}
