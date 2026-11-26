export module sst_schema;

import std;

export namespace schema {

inline constexpr std::string_view LatitudeVar = "latitude";
inline constexpr std::string_view LongitudeVar = "longitude";
inline constexpr std::string_view SstVar = "analysed_sst";
inline constexpr std::string_view TimeVar = "time";

inline constexpr std::string_view PixelDim = "pixel";
inline constexpr std::string_view TimeDim = "time";

inline constexpr std::string_view FillValueAtt = "_FillValue";
inline constexpr std::string_view UnitsAtt = "units";

inline constexpr std::string_view TimeUnits = "days since 1970-01-01";
inline constexpr std::string_view TimeCalendar = "proleptic_gregorian";

} // namespace schema
