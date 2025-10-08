import os
from yaep_python import Grammar
from yaep_python import _cffi


def test_parse_valid_description():
    g = Grammar()
    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0
    g.free()


def test_parse_invalid_utf8_description():
        """
        This test verifies YAEP's UTF-8 validator when given malformed byte sequences.

        Background / rationale:
        - YAEP expects UTF-8 encoded grammar descriptions. It validates incoming bytes and
            reports YAEP_INVALID_UTF8 if the input contains invalid UTF-8 sequences.

        - In Python, text is represented as Unicode `str` (sequence of code points). You cannot
            represent arbitrary invalid UTF-8 bytes directly as a `str` without decoding them
            through some codec (for example, 'latin1'). If you decode bytes containing an invalid
            UTF-8 lead byte using 'latin1' and then re-encode to UTF-8, the original malformed
            byte sequence will be transformed into a valid UTF-8 sequence (and YAEP will not see
            the original invalid bytes). That would defeat the purpose of this test.

        - Therefore the wrapper exposes `parse_description_bytes(buf: bytes)` which passes the
            provided bytes unchanged to YAEP. This allows the test to inject a truly malformed
            byte sequence and confirm YAEP detects it.

        Test behavior:
        - We pass a buffer ending with 0xC3 (a UTF-8 lead byte that lacks its required
            continuation). YAEP should return the YAEP_INVALID_UTF8 condition; the Python
            wrapper now translates that into a `YaepInvalidUTF8Error` exception with YAEP's
            diagnostic message.
        """

        from yaep_python import YaepInvalidUTF8Error

        g = Grammar()
        # invalid trailing continuation byte (single 0xC3)
        invalid = b"TERM;\nS : 'a' S 'b' | ;\n\xC3"

        # We expect the wrapper to raise a rich Python exception for invalid UTF-8,
        # because it maps YAEP_INVALID_UTF8 to YaepInvalidUTF8Error.
        try:
                g.parse_description_bytes(invalid, strict=False)
                # If no exception raised, that's unexpected — fail the test explicitly
                assert False, "Expected YaepInvalidUTF8Error"
        except YaepInvalidUTF8Error as e:
                msg = str(e)
                assert 'invalid UTF-8' in msg
        finally:
                g.free()
