#!/usr/bin/env python3
"""
Regex vs YAEP Grammar Example: String Format Validation

This example demonstrates two approaches to validating string formats:
1. Regular expressions (regex) - pattern matching
2. YAEP grammars - formal grammar parsing

We show phone number and email validation as examples.
"""

import re
from yaep_python import Grammar

def regex_validation_examples():
    """Demonstrate regex-based validation."""
    print("=== REGEX VALIDATION EXAMPLES ===\n")

    # Phone number regex (simplified version)
    phone_regex = r"^(\(\d{3}\)\s?|\d{3}[-\s]?)?\d{3}[-\s]?\d{4}$"

    # Email regex (simplified version)
    email_regex = r"^[\w.-]+@[\w.-]+\.[a-zA-Z]{2,4}$"

    test_phones = [
        "123-456-7890",      # Valid: XXX-XXX-XXXX
        "(123) 456-7890",    # Valid: (XXX) XXX-XXXX
        "1234567890",        # Valid: XXXXXXXXXX
        "123 456 7890",      # Valid: XXX XXX XXXX
        "123-456-789",       # Invalid: too few digits
        "abc-def-ghij",      # Invalid: non-digits
        "123-456-78901",     # Invalid: too many digits
    ]

    test_emails = [
        "user@example.com",      # Valid
        "user.name@domain.co.uk", # Valid
        "user+tag@gmail.com",    # Valid (simplified regex allows this)
        "user@",                 # Invalid: no domain
        "@domain.com",           # Invalid: no local part
        "user@.com",             # Invalid: domain starts with dot
        "user@domain",           # Invalid: no TLD
    ]

    print("Phone Number Validation:")
    for phone in test_phones:
        match = re.match(phone_regex, phone)
        status = "✓ VALID" if match else "✗ INVALID"
        print(f"  {phone:<20} {status}")

    print("\nEmail Validation:")
    for email in test_emails:
        match = re.match(email_regex, email)
        status = "✓ VALID" if match else "✗ INVALID"
        print(f"  {email:<25} {status}")

def yaep_validation_examples():
    """Demonstrate YAEP grammar-based validation."""
    print("\n\n=== YAEP GRAMMAR VALIDATION EXAMPLES ===\n")

    # Phone number grammar using character literals
    phone_grammar = """
TERM;

phone : area_code exchange line
      | exchange line
      ;

area_code : '(' '1' '2' '3' ')'
          | '1' '2' '3'
          ;

exchange : '4' '5' '6' separator
         | '4' '5' '6'
         ;

line : '7' '8' '9' '0' ;

separator : '-'
          | ' '
          ;
"""

    # Email grammar using character literals (simplified)
    email_grammar = """
TERM;

email : local '@' domain ;

local : letter local_rest
      | digit local_rest
      ;
local_rest : letter local_rest
           | digit local_rest
           | '_' local_rest
           | '-' local_rest
           | '.' local_rest
           |
           ;

domain : domain_part '.' tld ;
domain_part : letter domain_rest
            | digit domain_rest
            ;
domain_rest : letter domain_rest
            | digit domain_rest
            | '-' domain_rest
            | '.' domain_rest
            |
            ;

tld : letter tld_rest ;
tld_rest : letter tld_rest
         | digit tld_rest
         |
         ;

letter : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm'
       | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'
       | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M'
       | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z'
       ;

digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""

    # Test phone numbers
    test_phones = [
        "123-456-7890",      # Should parse
        "(123) 456-7890",    # Should parse
        "1234567890",        # Should parse
        "123 456 7890",      # Should parse
        "123-456-789",       # Should fail
        "abc-def-ghij",      # Should fail
    ]

    # Test emails
    test_emails = [
        "user@example.com",  # Should parse
        "a@b.co",           # Should parse
        "user@",            # Should fail
        "@domain.com",      # Should fail
        "user@domain",      # Should fail
    ]

    print("Phone Number Grammar Validation:")
    validate_with_grammar(phone_grammar, test_phones, "phone")

    print("\nEmail Grammar Validation:")
    validate_with_grammar(email_grammar, test_emails, "email")

def validate_with_grammar(grammar_text, test_strings, start_symbol):
    """Helper function to validate strings using a YAEP grammar."""
    try:
        grammar = Grammar()
        rc = grammar.parse_description(grammar_text)
        if rc != 0:
            print(f"  Error parsing grammar: {grammar.error_message()}")
            return

        for test_str in test_strings:
            # Convert string to token stream (each char as ord value)
            tokens = [ord(c) for c in test_str]

            rc, tree, syntax_err = grammar.parse(tokens)

            if rc == 0 and tree is not None:
                status = "✓ PARSED"
            else:
                status = "✗ FAILED"

            print(f"  {test_str:<20} {status}")

    except Exception as e:
        print(f"  Error: {e}")

def main():
    """Main demonstration function."""
    print("YAEP Grammar vs Regex: String Format Validation")
    print("=" * 50)

    regex_validation_examples()
    yaep_validation_examples()

    print("\n" + "=" * 50)
    print("SUMMARY:")
    print("- Regex: Fast, concise patterns for simple validation")
    print("- YAEP Grammar: Formal parsing, can extract structure,")
    print("  handle complex relationships, provide detailed errors")
    print("- Choose based on complexity and requirements!")

if __name__ == "__main__":
    main()