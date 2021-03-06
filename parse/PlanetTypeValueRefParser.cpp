#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct planet_type_parser_rules :
        public parse::detail::enum_value_ref_rules<PlanetType>
    {
        planet_type_parser_rules() :
            enum_value_ref_rules("PlanetType")
        {
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetType_
                |    tok.OriginalType_
                |    tok.NextCloserToOriginalPlanetType_
                |    tok.NextBetterPlanetType_
                |    tok.ClockwiseNextPlanetType_
                |    tok.CounterClockwiseNextPlanetType_
                ;

            enum_expr
                =   tok.Swamp_      [ _val = PT_SWAMP ]
                |   tok.Toxic_      [ _val = PT_TOXIC ]
                |   tok.Inferno_    [ _val = PT_INFERNO ]
                |   tok.Radiated_   [ _val = PT_RADIATED ]
                |   tok.Barren_     [ _val = PT_BARREN ]
                |   tok.Tundra_     [ _val = PT_TUNDRA ]
                |   tok.Desert_     [ _val = PT_DESERT ]
                |   tok.Terran_     [ _val = PT_TERRAN ]
                |   tok.Ocean_      [ _val = PT_OCEAN ]
                |   tok.Asteroids_  [ _val = PT_ASTEROIDS ]
                |   tok.GasGiant_   [ _val = PT_GASGIANT ]
                ;
        }
    };
}


namespace parse { namespace detail {

enum_value_ref_rules<PlanetType>& planet_type_rules()
{
    static planet_type_parser_rules retval;
    return retval;
}

} }
