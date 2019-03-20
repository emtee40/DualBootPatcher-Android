/*
 * Copyright (C) 2014-2019  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of DualBootPatcher
 *
 * DualBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DualBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DualBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mbutil/hash.h"

#include <memory>

#include <cstdio>

#include <sodium/crypto_hash_sha512.h>

#include "mbcommon/error_code.h"
#include "mbcommon/type_traits.h"


namespace mb::util
{

using ScopedFILE = std::unique_ptr<FILE, TypeFn<fclose>>;

/*!
 * \brief Compute SHA512 hash of a file
 *
 * \param path Path to file
 *
 * \return The digest on success or the error code on failure
 */
oc::result<Sha512Digest> sha512_hash(const std::string &path)
{
    ScopedFILE fp(fopen(path.c_str(), "rbe"));
    if (!fp) {
        return ec_from_errno();
    }

    std::array<unsigned char, 10240> buf;
    size_t n;

    crypto_hash_sha512_state s;
    if (crypto_hash_sha512_init(&s) != 0) {
        return std::errc::io_error;
    }

    while ((n = fread(buf.data(), 1, buf.size(), fp.get())) > 0) {
        if (crypto_hash_sha512_update(&s, buf.data(), n) != 0) {
            return std::errc::io_error;
        }
        if (n < buf.size()) {
            break;
        }
    }

    if (ferror(fp.get())) {
        return ec_from_errno();
    }

    Sha512Digest digest;

    if (crypto_hash_sha512_final(&s, digest.data()) != 0) {
        return std::errc::io_error;
    }

    return std::move(digest);
}

}
