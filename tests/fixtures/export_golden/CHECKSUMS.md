# Golden Master File Checksums (MD5)

**Generated:** 2026-02-07 18:24:27  
**Purpose:** Verify golden master file integrity

## Checksums

```
e9756fadb78206b6938470b68a5b3b78  golden_master.csv
3ab29ea0b751d2167ddcacbb817db98a  golden_master.json
3c67c6bc1b8683f225d99ebbf1db28a8  golden_master.xml
91b4efb0b05f88b269a54e526abec59e  golden_master.html
a9f5e1a763cff8f198c8dcaa6cf2850c  golden_master.tex
ca725ab4dc6fd5ea1f360c47b721cad1  golden_master.txt
```

## Verification

To verify file integrity after download or transfer:

```bash
cd tests/fixtures/export_golden
md5sum -c CHECKSUMS.md  # Linux
md5 -c CHECKSUMS.md     # macOS
```

Or manually compare:

```bash
md5sum golden_master.csv
# Should output: e9756fadb78206b6938470b68a5b3b78
```

## File Details

| File               | Size        | MD5 Checksum                       |
| ------------------ | ----------- | ---------------------------------- |
| golden_master.csv  | 945 bytes   | `e9756fadb78206b6938470b68a5b3b78` |
| golden_master.json | 1,237 bytes | `3ab29ea0b751d2167ddcacbb817db98a` |
| golden_master.xml  | 1,071 bytes | `3c67c6bc1b8683f225d99ebbf1db28a8` |
| golden_master.html | 1,087 bytes | `91b4efb0b05f88b269a54e526abec59e` |
| golden_master.tex  | 764 bytes   | `a9f5e1a763cff8f198c8dcaa6cf2850c` |
| golden_master.txt  | 1,549 bytes | `ca725ab4dc6fd5ea1f360c47b721cad1` |

## Notes

- These checksums are specific to the exact binary content of the files
- Any whitespace or line ending changes will alter the checksum
- Use structural comparison for validation, not just checksums
- Timestamps in files will differ on regeneration
