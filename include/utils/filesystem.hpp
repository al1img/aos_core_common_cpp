/*
 * Copyright (C) 2024 Renesas Electronics Corporation.
 * Copyright (C) 2024 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef UTILS_FILESYSTEM_HPP
#define UTILS_FILESYSTEM_HPP

#include <string>

#include <aos/common/tools/error.hpp>

namespace aos::common::utils {
/**
 * Creates a temporary directory using mkdtemp.
 *
 * @param dir Directory path where the temporary directory will be created.
 *            If empty, the system's temp directory will be used.
 * @param pattern Directory name pattern. The pattern must end with ".XXXXXX",
 *                where each 'X' will be replaced by a random character.
 *                If the pattern doesn't end with ".XXXXXX", it will be appended.
 *                Examples:
 *                - "tempdir.XXXXXX" might result in "tempdir.a1b2c3"
 *                - "mydir_" will become "mydir_.XXXXXX" and might result in "mydir_.a1b2c3"
 *                - If empty, "tmp.XXXXXX" will be used
 * @return RetWithError<std::string> containing the path of the created directory
 *         on success, or an error message on failure.
 */
RetWithError<std::string> MkTmpDir(const std::string& dir = "", const std::string& pattern = "");

} // namespace aos::common::utils

#endif // UTILS_FILESYSTEM_HPP
