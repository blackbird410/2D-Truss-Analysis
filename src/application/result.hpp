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
 * @brief Handle type for managing truss instances.
 *
 * Provides an opaque numeric reference to internal truss storage without
 * exposing implementation details to the Interface layer.
 */
using TrussHandle = size_t;

/**
 * @brief Result type for operations that may fail.
 *
 * Encapsulates success/failure state with error messaging.
 * Provides a monadic interface for error propagation without exceptions.
 *
 * @tparam T Value type returned on success.
 */
template <typename T>
struct Result {
    bool success;
    T value;
    std::string errorMessage;

    /**
     * @brief Construct a successful result carrying a value.
     * @param val Value to return to the caller.
     * @return Result with success == true and the given value.
     */
    static Result<T> Success(T val) { return {true, std::move(val), ""}; }

    /**
     * @brief Construct a failure result carrying an error message.
     * @param msg Human-readable description of the error.
     * @return Result with success == false and an empty-constructed value.
     */
    static Result<T> Failure(const std::string& msg) { return {false, T{}, msg}; }

    /**
     * @brief Implicit conversion to bool for use in if-conditions.
     * @return true if the operation succeeded, false otherwise.
     */
    operator bool() const { return success; }
};

}  // namespace truss::application
