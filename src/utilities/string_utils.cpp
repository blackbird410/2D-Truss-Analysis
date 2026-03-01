#include "utilities/string_utils.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace truss::utils::string {

// ─── Formatting ──────────────────────────────────────────────────────────────

std::string formatReal(double value, int precision, bool scientific) {
    std::ostringstream oss;
    if (scientific) {
        oss << std::scientific;
    } else {
        oss << std::fixed;
    }
    oss << std::setprecision(precision) << value;
    return oss.str();
}

std::string formatTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_val = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t_val), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ─── Case conversion ─────────────────────────────────────────────────────────

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

// ─── Escaping ────────────────────────────────────────────────────────────────

std::string escapeJson(const std::string& text) {
    std::string result;
    result.reserve(text.length());

    for (char c : text) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b";  break;
            case '\f': result += "\\f";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Remaining control characters → \uXXXX
                    std::ostringstream oss;
                    oss << "\\u" << std::setw(4) << std::setfill('0') << std::hex
                        << std::nouppercase
                        << static_cast<int>(static_cast<unsigned char>(c));
                    result += oss.str();
                } else {
                    result += c;
                }
        }
    }
    return result;
}

std::string escapeXml(const std::string& text) {
    std::string result;
    result.reserve(text.length());

    for (char c : text) {
        switch (c) {
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '&':  result += "&amp;";  break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:
                if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
                    // Control characters (except tab, LF, CR)
                    std::ostringstream oss;
                    oss << "&#" << static_cast<int>(static_cast<unsigned char>(c)) << ";";
                    result += oss.str();
                } else {
                    result += c;
                }
        }
    }
    return result;
}

std::string escapeHtml(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        switch (c) {
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '&':  result += "&amp;";  break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&#39;";  break;
            default:   result += c;
        }
    }
    return result;
}

std::string escapeLatex(const std::string& text) {
    std::string result;
    result.reserve(static_cast<std::size_t>(text.size() * 1.2));

    for (char c : text) {
        switch (c) {
            case '\\': result += "\\textbackslash{}";   break;
            case '{':  result += "\\{";                 break;
            case '}':  result += "\\}";                 break;
            case '$':  result += "\\$";                 break;
            case '&':  result += "\\&";                 break;
            case '%':  result += "\\%";                 break;
            case '#':  result += "\\#";                 break;
            case '_':  result += "\\_";                 break;
            case '~':  result += "\\textasciitilde{}";  break;
            case '^':  result += "\\textasciicircum{}"; break;
            default:   result += c;
        }
    }
    return result;
}

}  // namespace truss::utils::string
