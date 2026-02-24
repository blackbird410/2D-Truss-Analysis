/**
 * @file result.hpp
 * @brief Result type for operations that may fail.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Provides a simple Result<T> monad for error handling without exceptions.
 *
 * Architecture: Application Layer Common Types
 */

#pragma once

#include <cstddef>
#include <string>
#include <utility>

namespace truss::application {

/**
 * @brief Handle type for managing truss instances
 *
 * Provides opaque reference to internal truss storage without
 * exposing implementation details to Interface layer.
 */
using TrussHandle = size_t;

/**
 * @brief Result type for operations that may fail
 *
 * Encapsulates success/failure state with error messaging.
 * Provides monadic interface for error propagation.
 */
template <typename T>
struct Result {
    bool success;
    T value;
    std::string errorMessage;

    static Result<T> Success(T val) { return {true, std::move(val), ""}; }
    static Result<T> Failure(const std::string& msg) { return {false, T{}, msg}; }

    operator bool() const { return success; }
};

}  // namespace truss::application
