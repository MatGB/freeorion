#ifndef _Effect_h_
#define _Effect_h_

#include "EffectAccounting.h"

#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>

class UniverseObject;
struct ScriptingContext;

namespace Condition {
    struct ConditionBase;
    typedef std::vector<std::shared_ptr<const UniverseObject>> ObjectSet;
}

namespace ValueRef {
    template <class T>
    struct ValueRefBase;
}

namespace Effect {
class EffectBase;


/** Contains one or more Effects, a Condition which indicates the objects in
  * the scope of the Effect(s), and a Condition which indicates whether or not
  * the Effect(s) will be executed on the objects in scope during the current
  * turn.  Since Conditions operate on sets of objects (usually all objects in
  * the universe), the activation condition bears some explanation.  It exists
  * to allow an EffectsGroup to be activated or suppressed based on the source
  * object only (the object to which the EffectsGroup is attached).  It does
  * this by considering the "universe" containing only the source object. If
  * the source object meets the activation condition, the EffectsGroup will be
  * active in the current turn. */
class FO_COMMON_API EffectsGroup {
public:
    EffectsGroup(Condition::ConditionBase* scope, Condition::ConditionBase* activation,
                 const std::vector<EffectBase*>& effects, const std::string& accounting_label = "",
                 const std::string& stacking_group = "", int priority = 0,
                 std::string description = "") :
        m_scope(scope),
        m_activation(activation),
        m_stacking_group(stacking_group),
        m_effects(effects),
        m_accounting_label(accounting_label),
        m_priority(priority),
        m_description(description)
    {}
    virtual ~EffectsGroup();

    //void    GetTargetSet(int source_id, TargetSet& targets) const;
    //void    GetTargetSet(int source_id, TargetSet& targets, const TargetSet& potential_targets) const;
    ///** WARNING: this GetTargetSet version will modify potential_targets.
    //  * in particular, it will move detected targets from potential_targets
    //  * to targets. Cast the second parameter to \c const \c TargetSet& in
    //  * order to leave potential_targets unchanged. */
    //void    GetTargetSet(int source_id, TargetSet& targets, TargetSet& potential_targets) const;

    /** execute all effects in group */
    void    Execute(const TargetsCauses& targets_causes,
                    AccountingMap* accounting_map = nullptr,
                    bool only_meter_effects = false,
                    bool only_appearance_effects = false,
                    bool include_empire_meter_effects = false,
                    bool only_generate_sitrep_effects = false) const;

    const std::string&              StackingGroup() const       { return m_stacking_group; }
    Condition::ConditionBase*       Scope() const               { return m_scope; }
    Condition::ConditionBase*       Activation() const          { return m_activation; }
    const std::vector<EffectBase*>& EffectsList() const         { return m_effects; }
    const std::string&              GetDescription() const;
    const std::string&              AccountingLabel() const     { return m_accounting_label; }
    int                             Priority() const            { return m_priority; }
    std::string                     Dump() const;
    bool                            HasMeterEffects() const;
    bool                            HasAppearanceEffects() const;
    bool                            HasSitrepEffects() const;

    void                            SetTopLevelContent(const std::string& content_name);

    virtual unsigned int            GetCheckSum() const;

protected:
    Condition::ConditionBase*   m_scope;
    Condition::ConditionBase*   m_activation;
    std::string                 m_stacking_group;
    std::vector<EffectBase*>    m_effects;
    std::string                 m_accounting_label;
    int                         m_priority;
    std::string                 m_description;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns a single string which `Dump`s a vector of EffectsGroups. */
FO_COMMON_API std::string Dump(const std::vector<std::shared_ptr<EffectsGroup>>& effects_groups);

/** The base class for all Effects.  When an Effect is executed, the source
  * object (the object to which the Effect or its containing EffectGroup is
  * attached) and the target object are both required.  Note that this means
  * that ValueRefs contained within Effects can refer to values in either the
  * source or target objects. */
class FO_COMMON_API EffectBase {
public:
    virtual ~EffectBase();

    virtual void Execute(const ScriptingContext& context) const = 0;

    virtual void Execute(const ScriptingContext& context, const TargetSet& targets) const;

    virtual void Execute(const TargetsCauses& targets_causes,
                         AccountingMap* accounting_map = nullptr,
                         bool only_meter_effects = false,
                         bool only_appearance_effects = false,
                         bool include_empire_meter_effects = false,
                         bool only_generate_sitrep_effects = false) const;

    virtual std::string Dump() const = 0;

    virtual bool IsMeterEffect() const
    { return false; }

    virtual bool IsEmpireMeterEffect() const
    { return false; }

    virtual bool IsAppearanceEffect() const
    { return false; }

    virtual bool IsSitrepEffect() const
    { return false; }

    virtual bool IsConditionalEffect() const
    { return false; }

    virtual void SetTopLevelContent(const std::string& content_name) = 0;

    virtual unsigned int GetCheckSum() const;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Does nothing when executed. Useful for triggering side-effects of effect
  * execution without modifying the gamestate. */
class FO_COMMON_API NoOp : public EffectBase {
public:
    NoOp();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override
    {}

    unsigned int GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the meter of the given kind to \a value.  The max value of the meter
  * is set if \a max == true; otherwise the current value of the meter is set.
  * If the target of the Effect does not have the requested meter, nothing is
  * done. */
class FO_COMMON_API SetMeter : public EffectBase {
public:

    SetMeter(MeterType meter, ValueRef::ValueRefBase<double>* value);
    SetMeter(MeterType meter, ValueRef::ValueRefBase<double>* value, const std::string& accounting_label);

    virtual ~SetMeter();

    void Execute(const ScriptingContext& context) const override;

    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsMeterEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    MeterType GetMeterType() const
    { return m_meter; };

    const std::string&  AccountingLabel() const
    { return m_accounting_label; }

    unsigned int GetCheckSum() const override;

private:
    MeterType m_meter;
    ValueRef::ValueRefBase<double>* m_value;
    std::string m_accounting_label;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the indicated meter on all ship parts in the indicated subset.  This
  * has no effect on non-Ship targets.  If slot_type is specified, only parts
  * that can mount in the indicated slot type (internal or external) are
  * affected (this is not the same at the slot type in which the part is
  * actually located, as a part might be mountable in both types, and
  * located in a different type than specified, and would be matched). */
class FO_COMMON_API SetShipPartMeter : public EffectBase {
public:
    /** Affects the \a meter_type meter that belongs to part(s) named \a
        part_name. */
    SetShipPartMeter(MeterType meter_type,
                     ValueRef::ValueRefBase<std::string>* part_name,
                     ValueRef::ValueRefBase<double>* value);

    virtual ~SetShipPartMeter();

    void Execute(const ScriptingContext& context) const override;

    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsMeterEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef::ValueRefBase<std::string>* GetPartName() const
    { return m_part_name; }

    MeterType GetMeterType() const
    { return m_meter; }

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_part_name;
    MeterType m_meter;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the indicated meter on the empire with the indicated id to the
  * indicated value.  If \a meter is not a valid meter for empires,
  * does nothing. */
class FO_COMMON_API SetEmpireMeter : public EffectBase {
public:
    SetEmpireMeter(const std::string& meter, ValueRef::ValueRefBase<double>* value);

    SetEmpireMeter(ValueRef::ValueRefBase<int>* empire_id, const std::string& meter,
                   ValueRef::ValueRefBase<double>* value);

    virtual ~SetEmpireMeter();

    void Execute(const ScriptingContext& context) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsMeterEffect() const override
    { return true; }

    bool IsEmpireMeterEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<int>* m_empire_id;
    std::string m_meter;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the empire stockpile of the target's owning empire to \a value.  If
  * the target does not have exactly one owner, nothing is done. */
class FO_COMMON_API SetEmpireStockpile : public EffectBase {
public:
    SetEmpireStockpile(ResourceType stockpile,
                       ValueRef::ValueRefBase<double>* value);

    SetEmpireStockpile(ValueRef::ValueRefBase<int>* empire_id,
                       ResourceType stockpile,
                       ValueRef::ValueRefBase<double>* value);

    virtual ~SetEmpireStockpile();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<int>* m_empire_id;
    ResourceType m_stockpile;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Makes the target planet the capital of its owner's empire.  If the target
  * object is not a planet, does not have an owner, or has more than one owner
  * the effect does nothing. */
class FO_COMMON_API SetEmpireCapital : public EffectBase {
public:
    explicit SetEmpireCapital();

    explicit SetEmpireCapital(ValueRef::ValueRefBase<int>* empire_id);

    virtual ~SetEmpireCapital();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets.  Note that changing the
    type of a PT_ASTEROID or PT_GASGIANT planet will also change its size to SZ_TINY or SZ_HUGE, respectively.
    Similarly, changing type to PT_ASTEROID or PT_GASGIANT will also cause the size to change to SZ_ASTEROID or
    SZ_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetType : public EffectBase {
public:
    explicit SetPlanetType(ValueRef::ValueRefBase<PlanetType>* type);

    virtual ~SetPlanetType();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<PlanetType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet size of the target to \a size.  This has no effect on non-
  * Planet targets.  Note that changing the size of a PT_ASTEROID or PT_GASGIANT
  * planet will also change its type to PT_BARREN.  Similarly, changing size to
  * SZ_ASTEROID or SZ_GASGIANT will also cause the type to change to PT_ASTEROID
  * or PT_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetSize : public EffectBase {
public:
    explicit SetPlanetSize(ValueRef::ValueRefBase<PlanetSize>* size);

    virtual ~SetPlanetSize();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<PlanetSize>* m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the species on the target to \a species_name.  This works on planets
  * and ships, but has no effect on other objects. */
class FO_COMMON_API SetSpecies : public EffectBase {
public:
    explicit SetSpecies(ValueRef::ValueRefBase<std::string>* species);

    virtual ~SetSpecies();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_species_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets empire \a empire_id as the owner of the target.  This has no effect if
  * \a empire_id was already the owner of the target object. */
class FO_COMMON_API SetOwner : public EffectBase {
public:
    explicit SetOwner(ValueRef::ValueRefBase<int>* empire_id);

    virtual ~SetOwner();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a species for empire with id \a empire_id to
  * \a opinion */
class FO_COMMON_API SetSpeciesEmpireOpinion : public EffectBase {
public:
    SetSpeciesEmpireOpinion(ValueRef::ValueRefBase<std::string>* species_name,
                            ValueRef::ValueRefBase<int>* empire_id,
                            ValueRef::ValueRefBase<double>* opinion);

    virtual ~SetSpeciesEmpireOpinion();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_species_name;
    ValueRef::ValueRefBase<int>* m_empire_id;
    ValueRef::ValueRefBase<double>* m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a opinionated_species for other species
  * \a rated_species to \a opinion */
class FO_COMMON_API SetSpeciesSpeciesOpinion : public EffectBase {
public:
    SetSpeciesSpeciesOpinion(ValueRef::ValueRefBase<std::string>* opinionated_species_name,
                             ValueRef::ValueRefBase<std::string>* rated_species_name,
                             ValueRef::ValueRefBase<double>* opinion);

    virtual ~SetSpeciesSpeciesOpinion();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_opinionated_species_name;
    ValueRef::ValueRefBase<std::string>* m_rated_species_name;
    ValueRef::ValueRefBase<double>* m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Planet with specified \a type and \a size at the system with
  * specified \a location_id */
class FO_COMMON_API CreatePlanet : public EffectBase {
public:
    CreatePlanet(ValueRef::ValueRefBase<PlanetType>* type,
                 ValueRef::ValueRefBase<PlanetSize>* size,
                 ValueRef::ValueRefBase<std::string>* name = nullptr,
                 const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    virtual ~CreatePlanet();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<PlanetType>* m_type;
    ValueRef::ValueRefBase<PlanetSize>* m_size;
    ValueRef::ValueRefBase<std::string>* m_name;
    std::vector<EffectBase*> m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Building with specified \a type on the \a target Planet. */
class FO_COMMON_API CreateBuilding : public EffectBase {
public:
    explicit CreateBuilding(ValueRef::ValueRefBase<std::string>* building_type_name,
                            ValueRef::ValueRefBase<std::string>* name = nullptr,
                            const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    virtual ~CreateBuilding();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_building_type_name;
    ValueRef::ValueRefBase<std::string>* m_name;
    std::vector<EffectBase*> m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Ship with specified \a predefined_ship_design_name design
  * from those in the list of PredefinedShipDesignManager, and owned by the
  * empire with the specified \a empire_id */
class FO_COMMON_API CreateShip : public EffectBase {
public:
    explicit CreateShip(ValueRef::ValueRefBase<std::string>* predefined_ship_design_name,
                        ValueRef::ValueRefBase<int>* empire_id = nullptr,
                        ValueRef::ValueRefBase<std::string>* species_name = nullptr,
                        ValueRef::ValueRefBase<std::string>* ship_name = nullptr,
                        const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    explicit CreateShip(ValueRef::ValueRefBase<int>* ship_design_id,
                        ValueRef::ValueRefBase<int>* empire_id = nullptr,
                        ValueRef::ValueRefBase<std::string>* species_name = nullptr,
                        ValueRef::ValueRefBase<std::string>* ship_name = nullptr,
                        const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    virtual ~CreateShip();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_design_name;
    ValueRef::ValueRefBase<int>* m_design_id;
    ValueRef::ValueRefBase<int>* m_empire_id;
    ValueRef::ValueRefBase<std::string>* m_species_name;
    ValueRef::ValueRefBase<std::string>* m_name;
    std::vector<EffectBase*> m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Field with specified \a field_type_name FieldType
  * of the specified \a size. */
class FO_COMMON_API CreateField : public EffectBase {
public:
    explicit CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                         ValueRef::ValueRefBase<double>* size = nullptr,
                         ValueRef::ValueRefBase<std::string>* name = nullptr,
                         const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                ValueRef::ValueRefBase<double>* x,
                ValueRef::ValueRefBase<double>* y,
                ValueRef::ValueRefBase<double>* size = nullptr,
                ValueRef::ValueRefBase<std::string>* name = nullptr,
                const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    virtual ~CreateField();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_field_type_name;
    ValueRef::ValueRefBase<double>* m_x;
    ValueRef::ValueRefBase<double>* m_y;
    ValueRef::ValueRefBase<double>* m_size;
    ValueRef::ValueRefBase<std::string>* m_name;
    std::vector<EffectBase*> m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new system with the specified \a colour and at the specified
  * location. */
class FO_COMMON_API CreateSystem : public EffectBase {
public:
    CreateSystem(ValueRef::ValueRefBase< ::StarType>* type,
                 ValueRef::ValueRefBase<double>* x,
                 ValueRef::ValueRefBase<double>* y,
                 ValueRef::ValueRefBase<std::string>* name = nullptr,
                 const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    CreateSystem(ValueRef::ValueRefBase<double>* x,
                 ValueRef::ValueRefBase<double>* y,
                 ValueRef::ValueRefBase<std::string>* name = nullptr,
                 const std::vector<EffectBase*>& effects_to_apply_after = std::vector<EffectBase*>());

    virtual ~CreateSystem();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase< ::StarType>* m_type;
    ValueRef::ValueRefBase<double>* m_x;
    ValueRef::ValueRefBase<double>* m_y;
    ValueRef::ValueRefBase<std::string>* m_name;
    std::vector<EffectBase*> m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Destroys the target object.  When executed on objects that contain other
  * objects (such as Fleets and Planets), all contained objects are destroyed
  * as well.  Destroy effects delay the desctruction of their targets until
  * after other all effects have executed, to ensure the source or target of
  * other effects are present when they execute. */
class FO_COMMON_API Destroy : public EffectBase {
public:
    Destroy();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override
    {}

    unsigned int GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Adds the Special with the name \a name to the target object. */
class FO_COMMON_API AddSpecial : public EffectBase {
public:
    explicit AddSpecial(const std::string& name, float capacity = 1.0f);

    explicit AddSpecial(ValueRef::ValueRefBase<std::string>* name,
                        ValueRef::ValueRefBase<double>* capacity = nullptr);

    ~AddSpecial();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef::ValueRefBase<std::string>*  GetSpecialName() const
    { return m_name; }

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_name;
    ValueRef::ValueRefBase<double>* m_capacity;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes the Special with the name \a name to the target object.  This has
  * no effect if no such Special was already attached to the target object. */
class FO_COMMON_API RemoveSpecial : public EffectBase {
public:
    explicit RemoveSpecial(const std::string& name);

    explicit RemoveSpecial(ValueRef::ValueRefBase<std::string>* name);

    ~RemoveSpecial();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API AddStarlanes : public EffectBase {
public:
    explicit AddStarlanes(Condition::ConditionBase* other_lane_endpoint_condition);

    virtual ~AddStarlanes();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    Condition::ConditionBase* m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API RemoveStarlanes : public EffectBase {
public:
    explicit RemoveStarlanes(Condition::ConditionBase* other_lane_endpoint_condition);

    virtual ~RemoveStarlanes();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    Condition::ConditionBase* m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the star type of the target to \a type.  This has no effect on
  * non-System targets. */
class FO_COMMON_API SetStarType : public EffectBase {
public:
    explicit SetStarType(ValueRef::ValueRefBase<StarType>* type);

    virtual ~SetStarType();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<StarType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location of another UniverseObject that matches
  * the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API MoveTo : public EffectBase {
public:
    explicit MoveTo(Condition::ConditionBase* location_condition);

    virtual ~MoveTo();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    Condition::ConditionBase* m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location as though it was moving in orbit of
  * some object or position on the map.  Sign of \a speed indicates CCW / CW
  * rotation.*/
class FO_COMMON_API MoveInOrbit : public EffectBase {
public:
    MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                Condition::ConditionBase* focal_point_condition);

    MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                ValueRef::ValueRefBase<double>* focus_x = nullptr,
                ValueRef::ValueRefBase<double>* focus_y = nullptr);

    virtual ~MoveInOrbit();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<double>* m_speed;
    Condition::ConditionBase* m_focal_point_condition;
    ValueRef::ValueRefBase<double>* m_focus_x;
    ValueRef::ValueRefBase<double>* m_focus_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject a specified distance towards some object or
  * position on the map. */
class FO_COMMON_API MoveTowards : public EffectBase {
public:
    MoveTowards(ValueRef::ValueRefBase<double>* speed,
                Condition::ConditionBase* dest_condition);

    MoveTowards(ValueRef::ValueRefBase<double>* speed,
                ValueRef::ValueRefBase<double>* dest_x = nullptr,
                ValueRef::ValueRefBase<double>* dest_y = nullptr);

    virtual ~MoveTowards();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<double>* m_speed;
    Condition::ConditionBase* m_dest_condition;
    ValueRef::ValueRefBase<double>* m_dest_x;
    ValueRef::ValueRefBase<double>* m_dest_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the route of the target fleet to move to an UniverseObject that
  * matches the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API SetDestination : public EffectBase {
public:
    explicit SetDestination(Condition::ConditionBase* location_condition);

    virtual ~SetDestination();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    Condition::ConditionBase* m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets aggression level of the target object. */
class FO_COMMON_API SetAggression : public EffectBase {
public:
    explicit SetAggression(bool aggressive);

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override
    {}

    unsigned int GetCheckSum() const override;

private:
    bool m_aggressive;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Causes the owner empire of the target object to win the game.  If the
  * target object has multiple owners, nothing is done. */
class FO_COMMON_API Victory : public EffectBase {
public:
    explicit Victory(const std::string& reason_string); // TODO: Make this a ValueRefBase<std::string>*

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override
    {}

    unsigned int GetCheckSum() const override;

private:
    std::string m_reason_string;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets whether an empire has researched at tech, and how much research
  * progress towards that tech has been completed. */
class FO_COMMON_API SetEmpireTechProgress : public EffectBase {
public:
    SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                          ValueRef::ValueRefBase<double>* research_progress);

    SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                          ValueRef::ValueRefBase<double>* research_progress,
                          ValueRef::ValueRefBase<int>* empire_id);

    virtual ~SetEmpireTechProgress();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_tech_name;
    ValueRef::ValueRefBase<double>* m_research_progress;
    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API GiveEmpireTech : public EffectBase {
public:
    explicit GiveEmpireTech(ValueRef::ValueRefBase<std::string>* tech_name,
                            ValueRef::ValueRefBase<int>* empire_id = nullptr);

    virtual ~GiveEmpireTech();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    ValueRef::ValueRefBase<std::string>* m_tech_name;
    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Generates a sitrep message for the empire with id \a recipient_empire_id.
  * The message text is the user string specified in \a message_string with
  * string substitutions into the message text as specified in \a message_parameters
  * which are substituted as string parameters %1%, %2%, %3%, etc. in the order
  * they are specified.  Extra parameters beyond those needed by \a message_string
  * are ignored, and missing parameters are left as blank text. */
class FO_COMMON_API GenerateSitRepMessage : public EffectBase {
public:
    typedef std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*>> MessageParams;

    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          const MessageParams& message_parameters,
                          ValueRef::ValueRefBase<int>* recipient_empire_id,
                          EmpireAffiliationType affiliation,
                          const std::string label = "",
                          bool stringtable_lookup = true);

    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          const MessageParams& message_parameters,
                          EmpireAffiliationType affiliation,
                          Condition::ConditionBase* condition = nullptr,
                          const std::string label = "",
                          bool stringtable_lookup = true);

    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          const MessageParams& message_parameters,
                          EmpireAffiliationType affiliation,
                          const std::string& label = "",
                          bool stringtable_lookup = true);

    virtual ~GenerateSitRepMessage();

    void Execute(const ScriptingContext& context) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    bool IsSitrepEffect() const override
    { return true; }

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    const std::string& MessageString() const
    { return m_message_string; }

    const std::string& Icon() const
    { return m_icon; }

    MessageParams MessageParameters()
    { return m_message_parameters; }

    ValueRef::ValueRefBase<int>* RecipientID() const
    { return m_recipient_empire_id; }

    Condition::ConditionBase* GetCondition() const
    { return m_condition; }

    EmpireAffiliationType Affiliation() const
    { return m_affiliation; }

    unsigned int GetCheckSum() const override;

private:
    std::string m_message_string;
    std::string m_icon;
    MessageParams m_message_parameters;
    ValueRef::ValueRefBase<int>* m_recipient_empire_id;
    Condition::ConditionBase* m_condition;
    EmpireAffiliationType m_affiliation;
    std::string m_label;
    bool m_stringtable_lookup;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies an overlay texture to Systems. */
class FO_COMMON_API SetOverlayTexture : public EffectBase {
public:
    SetOverlayTexture(const std::string& texture, ValueRef::ValueRefBase<double>* size);

    virtual ~SetOverlayTexture();

    void Execute(const ScriptingContext& context) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsAppearanceEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    std::string m_texture;
    ValueRef::ValueRefBase<double>* m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies a texture to Planets. */
class FO_COMMON_API SetTexture : public EffectBase {
public:
    explicit SetTexture(const std::string& texture);

    void Execute(const ScriptingContext& context) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsAppearanceEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override
    {}

    unsigned int GetCheckSum() const override;

private:
    std::string m_texture;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets visibility of an object for an empire, independent of standard
  * visibility mechanics. */
class FO_COMMON_API SetVisibility : public EffectBase {
public:
    SetVisibility(Visibility vis, EmpireAffiliationType affiliation,
                  ValueRef::ValueRefBase<int>* empire_id = nullptr,
                  Condition::ConditionBase* of_objects = nullptr);    // if not specified, acts on target. if specified, acts on all matching objects

    virtual ~SetVisibility();

    void Execute(const ScriptingContext& context) const override;

    std::string Dump() const override;

    void SetTopLevelContent(const std::string& content_name) override;

    Visibility GetVisibility() const
    { return m_vis; }

    ValueRef::ValueRefBase<int>* EmpireID() const
    { return m_empire_id; }

    EmpireAffiliationType Affiliation() const
    { return m_affiliation; }

    Condition::ConditionBase* OfObjectsCondition() const
    { return m_condition; }

    unsigned int GetCheckSum() const override;

private:
    Visibility m_vis;
    ValueRef::ValueRefBase<int>* m_empire_id;
    EmpireAffiliationType m_affiliation;
    Condition::ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Executes a set of effects if an execution-time condition is met, or an
  * alterative set of effects if the condition is not met. */
class FO_COMMON_API Conditional : public EffectBase {
public:
    Conditional(Condition::ConditionBase* target_condition,
                const std::vector<EffectBase*>& true_effects,
                const std::vector<EffectBase*>& false_effects);

    void Execute(const ScriptingContext& context) const override;
    /** Note: executes all of the true or all of the false effects on each
        target, without considering any of the only_* type flags. */

    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump() const override;

    bool IsMeterEffect() const override;

    bool IsAppearanceEffect() const override;

    bool IsSitrepEffect() const override;

    bool IsConditionalEffect() const override
    { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

protected:
    void Execute(const ScriptingContext& context, const TargetSet& targets,
                 AccountingMap* accounting_map = nullptr,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const;

private:
    Condition::ConditionBase* m_target_condition; // condition to apply to each target object to determine which effects to execute
    std::vector<EffectBase*> m_true_effects;     // effects to execute if m_target_condition matches target object
    std::vector<EffectBase*> m_false_effects;    // effects to execute if m_target_condition does not match target object

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
template <class Archive>
void EffectsGroup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_scope)
        & BOOST_SERIALIZATION_NVP(m_activation)
        & BOOST_SERIALIZATION_NVP(m_stacking_group)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_description);
}

template <class Archive>
void EffectBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void NoOp::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase);
}

template <class Archive>
void SetMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value)
        & BOOST_SERIALIZATION_NVP(m_accounting_label);
}

template <class Archive>
void SetShipPartMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_part_name)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireStockpile::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_stockpile)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireCapital::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void SetPlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void SetPlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void SetSpecies::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_species_name);
}

template <class Archive>
void SetOwner::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void CreatePlanet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateBuilding::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_building_type_name)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateShip::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_design_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_species_name)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateField::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_field_type_name)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void Destroy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase);
}

template <class Archive>
void AddSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_capacity);
}

template <class Archive>
void RemoveSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void AddStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void RemoveStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void SetStarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void MoveTo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void MoveInOrbit::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_focal_point_condition)
        & BOOST_SERIALIZATION_NVP(m_focus_x)
        & BOOST_SERIALIZATION_NVP(m_focus_y);
}

template <class Archive>
void MoveTowards::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_dest_condition)
        & BOOST_SERIALIZATION_NVP(m_dest_x)
        & BOOST_SERIALIZATION_NVP(m_dest_y);
}

template <class Archive>
void SetDestination::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void SetAggression::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_aggressive);
}

template <class Archive>
void Victory::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_reason_string);
}

template <class Archive>
void SetEmpireTechProgress::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_research_progress)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void GiveEmpireTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void GenerateSitRepMessage::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_message_string)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_message_parameters)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_label)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup);
}

template <class Archive>
void SetOverlayTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void SetTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture);
}

template <class Archive>
void SetVisibility::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_vis)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Conditional::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_target_condition)
        & BOOST_SERIALIZATION_NVP(m_true_effects)
        & BOOST_SERIALIZATION_NVP(m_false_effects);
}
} // namespace Effect

#endif // _Effect_h_
