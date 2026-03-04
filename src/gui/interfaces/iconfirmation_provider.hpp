/**
 * @file iconfirmation_provider.hpp
 * @brief Interface for confirming destructive operations without modal dialogs in tests.
 *
 * Phase 5: IConfirmationProvider allows ProjectControllerV2 to ask for user
 * confirmation without coupling directly to QMessageBox, enabling headless
 * unit tests via AutoConfirmProvider.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include <QString>
#include <QMessageBox>

namespace truss::gui::interfaces {

/**
 * @brief Abstract confirmation gate for destructive operations.
 *
 * Implement this interface to control how user confirmation dialogs behave.
 * In production code use ModalConfirmProvider (delegates to QMessageBox);
 * in unit tests use AutoConfirmProvider (returns a fixed value immediately).
 */
class IConfirmationProvider {
public:
    virtual ~IConfirmationProvider() = default;

    /**
     * @brief Ask the user to confirm a potentially destructive action.
     * @param title    Dialog title / action name.
     * @param message  Human-readable explanation of the consequence.
     * @return true if the user confirmed, false if they cancelled.
     */
    virtual bool confirm(const QString& title, const QString& message) = 0;
};

/**
 * @brief Test double: always returns a fixed result without UI interaction.
 *
 * Construct with `true` (default) to confirm all operations automatically,
 * or with `false` to simulate the user always declining.
 */
class AutoConfirmProvider final : public IConfirmationProvider {
public:
    explicit AutoConfirmProvider(bool result = true) noexcept : m_result{result} {}

    bool confirm(const QString& /*title*/, const QString& /*message*/) override
    {
        return m_result;
    }

private:
    bool m_result;
};

/**
 * @brief Production implementation: delegates to QMessageBox::question.
 */
class ModalConfirmProvider final : public IConfirmationProvider {
public:
    bool confirm(const QString& title, const QString& message) override
    {
        return QMessageBox::question(nullptr, title, message,
                                     QMessageBox::Yes | QMessageBox::No)
               == QMessageBox::Yes;
    }
};

}  // namespace truss::gui::interfaces
