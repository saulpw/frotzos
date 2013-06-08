# eliFs

## Motivation

A profoundly simple archive of files with these characteristics:

  * each file must be linear and contiguous
  * each file should be uncompressed and unencrypted
  * file format easily reverse engineered
  * minimal and obvious metadata
  * can be used as an appendable filesystem in a live system
  * presumes block (page/sector) random-access (SSD or virtual disk)
 
## File Header 

(all multi-byte fields are little endian)

  * 4-byte app specific field (short jmp for boot sector)
  * 4-byte magic sync token: 'File' (little-endian, so reads "eliF")
  * 4-byte exact file length (not including padding or header)
  * 2 bytes reserved
  * 1 byte status: 0=sentinel, 1=existing, 2=writable for append, 4=deleted
  * 1-byte filename length in bytes, including zero-padding for alignment
  * utf8 filename (7-bit clean preferred), must be zero-terminated, should
    be padded to (at least) word boundary

### File length

  The file length is an exact byte count in little-endian order.  The size of
  this field is 4 bytes (4GB max file size).  [The next 2 bytes are reserved so
  the file size could be up to 256TB, but that seems like an unreasonable file
  size].

### 1-byte status field

  In a properly constructed/closed eliFs, all files have status 1 (existing)
  and the fs ends with a status 0 sentinel header (all other fields are 0
  except possibly the magic).  Thus the sentinel does not have to be stored
  if further reads give all zeroes.

  To unlink a file, rewrite the sector with its header so the status is set to
  4.

  While writing a file for append (unknown length), the sector containing the
  sentinel header is first written with a status of 2 (writing/append) and a
  file length of 0.
  
  When the appending file is closed/finalized, the final sector is written out
  with a new status 0 sentinel.  The sector containing the appending file
  header is rewritten with the correct file length and a status of 1
  (existing/closed).

  Only one 'appending' (status == 2) file can exist at a time and it must be at
  the end of the archive.  Deleted files may be reused to write smaller
  known-length files (or streamed files that are fully buffered first), with
  any remainder as another deleted file (this requires that the remainder be
  at least 16 bytes).
  
### Filename length and filename

   The filename length is the last (15th) byte of the header, and the filename
   follows directly.  The length must include at least 1 null byte and null
   bytes are ignored.  Thus filenames are both length-prefixed and
   null-terminated.

   If the filename length (N) is
      == 0      no filename
       > 0      exact count in bytes

### File contents

    The file contents directly follow the filename.  Thus if the header is at
    uint8_t* H, the contents are at H + 16 + H.namelength.

    The next file header begins at H + 16 + H.namelength + H.filelength.

# License

This specification for the eliFs file format was written by Saul Pwanson in
2013 and is released into the public domain for any and all purposes.


