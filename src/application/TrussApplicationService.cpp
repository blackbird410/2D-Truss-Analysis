/**
 * @file TrussApplicationService.cpp
 * @brief Implementation of truss model application service
 */

#include "TrussApplicationService.hpp"
#include <stdexcept>

namespace truss::application {

using namespace truss::core;
using namespace truss::infrastructure::io;

TrussApplicationService::TrussApplicationService() = default;

Result<TrussHandle> TrussApplicationService::createTruss(const std::string& name) {
    try {
        auto truss = std::make_shared<Truss>(name);
        TrussHandle handle = generateHandle();
        m_trusses[handle] = truss;
        return Result<TrussHandle>::Success(handle);
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(
            std::string("Failed to create truss: ") + e.what()
        );
    }
}

Result<TrussHandle> TrussApplicationService::loadTruss(const std::filesystem::path& filepath) {
    try {
        // Auto-detect format from extension using FileIOFactory
        auto format = FileIOFactory::detectFormat(filepath);
        if (format == FileFormat::Auto) {
            return Result<TrussHandle>::Failure(
                "Could not detect file format from extension: " + filepath.string()
            );
        }
        return loadTruss(filepath, format);
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(
            std::string("Failed to load truss: ") + e.what()
        );
    }
}

Result<TrussHandle> TrussApplicationService::loadTruss(
    const std::filesystem::path& filepath,
    FileFormat format) {
    
    try {
        // Create appropriate reader
        auto reader = FileIOFactory::createReader(format);
        
        // Read DTO from file
        FileIOOptions options;
        options.validateOnRead = false;  // We'll validate after assembly
        auto dto = reader->read(filepath, options);
        
        // Assemble Domain model from DTO
        auto truss = assembly::TrussAssembler::assembleTruss(dto);
        
        // Validate assembled truss
        auto validationResult = m_validator.validate(*truss);
        if (!validationResult.isValid()) {
            return Result<TrussHandle>::Failure(
                "Truss loaded but failed validation: " + validationResult.getSummary()
            );
        }
        
        // Store and return handle
        TrussHandle handle = generateHandle();
        m_trusses[handle] = truss;
        return Result<TrussHandle>::Success(handle);
        
    } catch (const FileIOException& e) {
        return Result<TrussHandle>::Failure(
            std::string("File I/O error: ") + e.what()
        );
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(
            std::string("Failed to load truss: ") + e.what()
        );
    }
}

Result<bool> TrussApplicationService::saveTruss(
    TrussHandle handle,
    const std::filesystem::path& filepath,
    bool overwrite) {
    
    try {
        // Auto-detect format from extension using FileIOFactory
        auto format = FileIOFactory::detectFormat(filepath);
        if (format == FileFormat::Auto) {
            return Result<bool>::Failure(
                "Could not detect file format from extension: " + filepath.string()
            );
        }
        return saveTruss(handle, filepath, format, overwrite);
    } catch (const std::exception& e) {
        return Result<bool>::Failure(
            std::string("Failed to save truss: ") + e.what()
        );
    }
}

Result<bool> TrussApplicationService::saveTruss(
    TrussHandle handle,
    const std::filesystem::path& filepath,
    FileFormat format,
    bool overwrite) {
    
    try {
        // Validate handle
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }
        
        auto truss = m_trusses[handle];
        
        // Validate truss before saving
        auto validationResult = m_validator.validate(*truss);
        if (!validationResult.isValid()) {
            return Result<bool>::Failure(
                "Cannot save invalid truss: " + validationResult.getSummary()
            );
        }
        
        // Create DTO from Domain model
        auto dto = assembly::TrussAssembler::createDTO(*truss);
        
        // Create appropriate writer
        auto writer = FileIOFactory::createWriter(format);
        
        // Write to file
        FileIOOptions options;
        options.overwriteExisting = overwrite;
        options.prettyPrint = true;
        options.includeMetadata = true;
        
        writer->write(dto, filepath, options);
        return Result<bool>::Success(true);
        
    } catch (const FileIOException& e) {
        return Result<bool>::Failure(
            std::string("File I/O error: ") + e.what()
        );
    } catch (const std::exception& e) {
        return Result<bool>::Failure(
            std::string("Failed to save truss: ") + e.what()
        );
    }
}

Result<validation::ValidationResult> TrussApplicationService::validateTruss(TrussHandle handle) {
    try {
        if (!isValidHandle(handle)) {
            return Result<validation::ValidationResult>::Failure("Invalid truss handle");
        }
        
        auto truss = m_trusses[handle];
        auto result = m_validator.validate(*truss);
        return Result<validation::ValidationResult>::Success(std::move(result));
        
    } catch (const std::exception& e) {
        return Result<validation::ValidationResult>::Failure(
            std::string("Validation failed: ") + e.what()
        );
    }
}

const core::interfaces::ITrussView& TrussApplicationService::getTrussView(TrussHandle handle) const {
    if (!isValidHandle(handle)) {
        throw std::invalid_argument("Invalid truss handle: " + std::to_string(handle));
    }
    return *m_trusses.at(handle);
}

core::Truss& TrussApplicationService::getTrussMutable(TrussHandle handle) {
    if (!isValidHandle(handle)) {
        throw std::invalid_argument("Invalid truss handle: " + std::to_string(handle));
    }
    return *m_trusses.at(handle);
}

bool TrussApplicationService::clearTruss(TrussHandle handle) {
    return m_trusses.erase(handle) > 0;
}

void TrussApplicationService::clearAll() {
    m_trusses.clear();
}

bool TrussApplicationService::isValidHandle(TrussHandle handle) const {
    return m_trusses.find(handle) != m_trusses.end();
}

} // namespace truss::application
