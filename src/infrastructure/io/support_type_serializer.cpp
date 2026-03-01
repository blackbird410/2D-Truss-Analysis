#include "infrastructure/io/support_type_serializer.hpp"

#include "infrastructure/io/io_types.hpp"

namespace truss::infrastructure::io {

core::SupportType parseSupportType(const std::string& str) {
    if (str == "free" || str == "Free")
        return core::SupportType::Free;
    if (str == "pinned" || str == "Pinned")
        return core::SupportType::Pinned;
    if (str == "fixed" || str == "Fixed")
        return core::SupportType::Pinned;
    if (str == "roller" || str == "Roller")
        return core::SupportType::RollerX;
    if (str == "pinned_x" || str == "PinnedX")
        return core::SupportType::RollerY;
    if (str == "pinned_y" || str == "PinnedY")
        return core::SupportType::RollerX;
    if (str == "roller_x" || str == "RollerX")
        return core::SupportType::RollerX;
    if (str == "roller_y" || str == "RollerY")
        return core::SupportType::RollerY;
    throw ParseException("Unknown support type: " + str);
}

std::string supportTypeToString(core::SupportType type) {
    switch (type) {
        case core::SupportType::Free:
            return "free";
        case core::SupportType::Pinned:
            return "pinned";
        case core::SupportType::RollerX:
            return "roller_x";
        case core::SupportType::RollerY:
            return "roller_y";
        default:
            return "free";
    }
}

}  // namespace truss::infrastructure::io
