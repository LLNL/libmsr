/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef LIBMSR_ERROR_H_INCLUDE
#define LIBMSR_ERROR_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NAME_MAX
#define NAME_MAX 1024
#endif

/// @brief Structure encompassing libmsr errors.
enum libmsr_error_e
{
    /// @brief Default error if value is 0 or -1.
    LIBMSR_ERROR_RUNTIME = -1,
    /// @brief Features not supported on a given platform (i.e.,
    /// MSR_TURBO_BOOST_RATIO, PP1 power domain).
    LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED = -2,
    /// @brief Accessing array index out of bounds.
    LIBMSR_ERROR_ARRAY_BOUNDS = -3,
    /// @brief MSR batch problems.
    LIBMSR_ERROR_MSR_BATCH = -4,
    /// @brief Platform hardware environment (i.e., num sockets, num cores, num
    /// threads).
    LIBMSR_ERROR_PLATFORM_ENV = -5,
    /// @brief Access msr or msr_batch modules.
    LIBMSR_ERROR_MSR_MODULE = -6,
    /// @brief Opening MSR file descriptors for reads/writes.
    LIBMSR_ERROR_MSR_OPEN = -7,
    /// @brief Closing MSR file descriptors.
    LIBMSR_ERROR_MSR_CLOSE = -8,
    /// @brief Reading MSRs.
    LIBMSR_ERROR_MSR_READ = -9,
    /// @brief Writing MSRs.
    LIBMSR_ERROR_MSR_WRITE = -10,
    /// @brief Initializing RAPL.
    LIBMSR_ERROR_RAPL_INIT = -11,
    /// @brief Initializing MSR.
    LIBMSR_ERROR_MSR_INIT = -12,
    /// @brief Invalid value in MSR.
    LIBMSR_ERROR_INVAL = -13,
    /// @brief Feature not yet implemented.
    LIBMSR_ERROR_NOT_IMPLEMENTED_YET = -14,
    /// @brief Memory allocation.
    LIBMSR_ERROR_MEMORY_ALLOCATION = -15,
    /// @brief Initializing CSRs.
    LIBMSR_ERROR_CSR_INIT = -16,
    /// @brief CSR counters.
    LIBMSR_ERROR_CSR_COUNTERS = -17,
};

/// @brief Display error message to user.
///
/// This is the generic libmsr error handling API. Use this function to report
/// when errors arise. If an error is not defined in libmsr_error_e, add
/// it to the aforementioned enum with the next sequential error code and
/// define its brief error message in the / source file.
///
/// @param [in] desc Extension to default libmsr error message.
///
/// @param [in] err libmsr_error_e error code, positive values are system
///        errors, negative values are libmsr errors. LIBMSR_ERROR_RUNTIME is
///        assumed if error code is 0.
///
/// @param [in] host Hostname environment variable.
///
/// @param [in] filename Name of source file where error occurred (use standard
///        predefined macro __FILE__).
///
/// @param [in] line Line number in source file where error occurred (use
///        standard predefined macro __LINE__).
void libmsr_error_handler(const char *desc,
                          int err,
                          const char *host,
                          const char *filename,
                          int line);

/// @brief Get brief libmsr error message based on error code.
///
/// @param [in] err libmsr_error_e error code, positive values are system
///        errors, negative values are libmsr errors. LIBMSR_ERROR_RUNTIME is
///        assumed if error code is 0.
///
/// @return Default libmsr error message.
char *get_libmsr_error_message(int err);

/// @brief Convert error code to brief libmsr error message.
///
/// @param [in] err libmsr_error_e error code, positive values are system
///        errors, negative values are libmsr errors. LIBMSR_ERROR_RUNTIME is
///        assumed if error code is 0.
///
/// @param [in] size Max length (in bytes) of brief libmsr error message.
///
/// @param [out] msg Brief libmsr error message.
void libmsr_error_message(int err,
                          char *msg,
                          size_t size);

#ifdef __cplusplus
}
#endif
#endif
