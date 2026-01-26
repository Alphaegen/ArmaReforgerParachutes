class ParachuteItemComponentClass : ScriptComponentClass {}

class ParachuteItemComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Parachute Prefab", "et")]
	protected ResourceName m_ParachutePrefab;

	// Durable, replicated state.
	[RplProp(onRplName: "OnRep_ParachuteUsed")]
	protected bool m_bParachuteUsed = false;

	protected InventoryItemComponent m_ItemComp;

	ResourceName GetParachutePrefab()
	{
		return m_ParachutePrefab;
	}

	bool GetParachuteUsed()
	{
		return m_bParachuteUsed;
	}

	// Server authoritative setter (clients should not change this)
	void SetParachuteUsed_Server()
	{
		if (!Replication.IsServer())
			return;

		if (m_bParachuteUsed)
			return;

		m_bParachuteUsed = true;
		Replication.BumpMe();
	}

	void OnRep_ParachuteUsed()
	{
		// Optional: UI refresh, disable action prompts, etc.
	}

	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		if (m_ParachutePrefab == "")
		{
			Print("ERROR: ParachuteItemComponent is missing its prefab! Skipping parachute registration.");
			return;
		}

		m_ItemComp = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (!m_ItemComp)
			return;

		m_ItemComp.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);

		GetGame().GetCallqueue().CallLater(PrimeCurrentSlot, 250, false);
	}

	override void OnDelete(IEntity owner)
	{
		if (m_ItemComp)
			m_ItemComp.m_OnParentSlotChangedInvoker.Remove(OnParentSlotChanged);
	}

	protected void PrimeCurrentSlot()
	{
		if (!m_ItemComp)
			return;

		InventoryStorageSlot slot = m_ItemComp.GetParentSlot();
		if (!slot)
			return;

		OnParentSlotChanged(null, slot);
	}

	void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		if (oldSlot)
		{
			ParachuteComponent parachuteComponent = ParachuteHelperFunctions.GetParachuteComponentFromSlotOwner(oldSlot);
			if (parachuteComponent && parachuteComponent.GetParachuteItem() == this)
				parachuteComponent.ClearParachuteItem();
		}

		if (newSlot)
		{
			ParachuteComponent parachuteComponent = ParachuteHelperFunctions.GetParachuteComponentFromSlotOwner(newSlot);
			if (parachuteComponent)
				parachuteComponent.SetParachuteItem(this);
		}
	}
}
