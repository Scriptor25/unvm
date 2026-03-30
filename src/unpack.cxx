#include <unvm/unvm.hxx>

#include <archive.h>
#include <archive_entry.h>

#include <iostream>

static la_ssize_t read_callback(archive */*arc*/, void *user_data, const void **buffer)
{
    static char buf[0x4000];

    const auto stream = static_cast<std::istream *>(user_data);

    stream->read(buf, sizeof(buf));
    const auto len = stream->gcount();

    if (len <= 0)
    {
        *buffer = nullptr;
        return 0;
    }

    *buffer = buf;
    return len;
}

int unvm::UnpackArchive(std::istream &stream, const std::filesystem::path &directory)
{
    const auto arc = archive_read_new();
    const auto ext = archive_write_disk_new();

    archive_read_support_format_all(arc);
    archive_read_support_filter_all(arc);

    archive_write_disk_set_options(
        ext,
        ARCHIVE_EXTRACT_TIME
        | ARCHIVE_EXTRACT_PERM
        | ARCHIVE_EXTRACT_ACL
        | ARCHIVE_EXTRACT_FFLAGS);

    if (const auto error = archive_read_open(arc, &stream, nullptr, read_callback, nullptr))
    {
        std::cerr << "failed to open archive: " << archive_error_string(arc) << std::endl;

        archive_read_free(arc);
        archive_write_free(ext);
        return error;
    }

    archive_entry *entry;
    const void *buf;
    std::size_t len;
    la_int64_t off;

    int err;
    while (!((err = archive_read_next_header(arc, &entry))))
    {
        auto pathname = directory / archive_entry_pathname(entry);
        auto pathname_string = pathname.string();
        archive_entry_set_pathname(entry, pathname_string.c_str());

        if (const auto error = archive_write_header(ext, entry))
        {
            std::cerr << "failed to write archive header: " << archive_error_string(ext) << std::endl;

            archive_read_free(arc);
            archive_write_free(ext);
            return error;
        }

        while (true)
        {
            if (const auto error = archive_read_data_block(arc, &buf, &len, &off))
            {
                if (error == ARCHIVE_EOF)
                {
                    break;
                }

                std::cerr << "failed to read archive data block: " << archive_error_string(arc) << std::endl;

                archive_read_free(arc);
                archive_write_free(ext);
                return error;
            }

            if (const auto error = archive_write_data_block(ext, buf, len, off))
            {
                std::cerr << "failed to write archive data block: " << archive_error_string(ext) << std::endl;

                archive_read_free(arc);
                archive_write_free(ext);
                return static_cast<int>(error);
            }
        }
    }

    if (err != ARCHIVE_EOF)
    {
        std::cerr << "failed to read archive header: " << archive_error_string(arc) << std::endl;

        archive_read_free(arc);
        archive_write_free(ext);
        return err;
    }

    archive_read_free(arc);
    archive_write_free(ext);
    return 0;
}
