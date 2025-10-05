#!/usr/bin/env python3
"""
Regex vs YAEP Grammar Example: String Format Validation

This example demonstrates two approaches to validating string formats:
1. Regular expressions (regex) - pattern matching
2. YAEP grammars - formal grammar parsing

We compare how regex and YAEP handle common validation tasks like:
- Phone numbers
- Email addresses  
- URLs
- Credit card numbers
- IPv4 addresses

The goal is to show that YAEP can be used for validation tasks where
you would normally use regex, with the added benefit of formal grammar
structure and the ability to extract/transform the parsed data.
"""

import os
import sys
import re

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def print_section(title):
    """Print a formatted section header."""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


def print_subsection(title):
    """Print a formatted subsection header."""
    print(f"\n{title}")
    print("-" * len(title))


# =============================================================================
# PHONE NUMBER VALIDATION
# =============================================================================

def phone_regex_validation():
    """Validate phone numbers using regex."""
    print_subsection("Phone Numbers - REGEX Approach")
    
    # Regex pattern for US phone numbers
    # Supports: (123) 456-7890, 123-456-7890, 123 456 7890, 1234567890, 456-7890
    phone_regex = r"^(\(\d{3}\)\s?|\d{3}[-\s]?)?\d{3}[-\s]?\d{4}$"
    
    test_cases = [
        ("123-456-7890", True),      # Valid: XXX-XXX-XXXX
        ("(123) 456-7890", True),    # Valid: (XXX) XXX-XXXX
        ("1234567890", True),        # Valid: XXXXXXXXXX (no separators)
        ("123 456 7890", True),      # Valid: XXX XXX XXXX
        ("456-7890", True),          # Valid: XXX-XXXX (no area code)
        ("123-456-789", False),      # Invalid: too few digits
        ("abc-def-ghij", False),     # Invalid: non-digits
        ("123-456-78901", False),    # Invalid: too many digits
    ]
    
    print(f"Pattern: {phone_regex}\n")
    for test_str, expected in test_cases:
        match = re.match(phone_regex, test_str)
        result = "VALID" if match else "INVALID"
        status = "✓" if (match is not None) == expected else "✗"
        print(f"  {status} {test_str:<20} → {result}")


def phone_yaep_validation():
    """Validate phone numbers using YAEP grammar."""
    print_subsection("Phone Numbers - YAEP Grammar Approach")
    
    # YAEP grammar for phone numbers
    # Note: This is a simplified grammar that matches specific patterns
    phone_grammar = """
TERM;

/* Main phone rule - with or without area code */
phone : area_code sep1 exchange sep2 line
      | exchange sep3 line
      ;

/* Area code can be (123) or just 123 */
area_code : '(' digit digit digit ')' space_opt
          | digit digit digit
          ;

/* Exchange is 3 digits */
exchange : digit digit digit ;

/* Line is 4 digits */
line : digit digit digit digit ;

/* Separators */
sep1 : '-' | ' ' | ;
sep2 : '-' | ' ' | ;
sep3 : '-' | ' ' | ;
space_opt : ' ' | ;

/* Digit enumeration */
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""
    
    test_cases = [
        ("123-456-7890", True),      # Valid
        ("(123) 456-7890", True),    # Valid
        ("1234567890", True),        # Valid
        ("123 456 7890", True),      # Valid
        ("456-7890", True),          # Valid
        ("123-456-789", False),      # Invalid: too few digits
        ("abc-def-ghij", False),     # Invalid: non-digits
        ("123-456-78901", False),    # Invalid: too many digits
    ]
    
    print("Grammar: US phone number patterns\n")
    validate_with_grammar(phone_grammar, test_cases)


# =============================================================================
# EMAIL VALIDATION
# =============================================================================

def email_regex_validation():
    """Validate email addresses using regex."""
    print_subsection("Email Addresses - REGEX Approach")
    
    # Simplified email regex
    email_regex = r"^[a-zA-Z0-9._+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$"
    
    test_cases = [
        ("user@example.com", True),          # Valid
        ("user.name@domain.co.uk", True),    # Valid
        ("user+tag@gmail.com", True),        # Valid
        ("user_123@test-site.org", True),    # Valid
        ("a@b.co", True),                    # Valid: minimal
        ("user@", False),                    # Invalid: no domain
        ("@domain.com", False),              # Invalid: no local part
        ("user@.com", False),                # Invalid: domain starts with dot
        ("user@domain", False),              # Invalid: no TLD
        ("user name@domain.com", False),     # Invalid: space in local
    ]
    
    print(f"Pattern: {email_regex}\n")
    for test_str, expected in test_cases:
        match = re.match(email_regex, test_str)
        result = "VALID" if match else "INVALID"
        status = "✓" if (match is not None) == expected else "✗"
        print(f"  {status} {test_str:<30} → {result}")


def email_yaep_validation():
    """Validate email addresses using YAEP grammar."""
    print_subsection("Email Addresses - YAEP Grammar Approach")
    
    email_grammar = """
TERM;

/* Email structure: local@domain.tld */
email : local '@' domain '.' tld ;

/* Local part: alphanumeric, dot, underscore, plus, hyphen */
local : local_char local_rest ;
local_rest : local_char local_rest | ;

local_char : letter | digit | '.' | '_' | '+' | '-' ;

/* Domain part: alphanumeric and hyphen/dot */
domain : domain_char domain_rest ;
domain_rest : domain_char domain_rest | ;

domain_char : letter | digit | '-' | '.' ;

/* TLD: letters and digits, must start with letter */
tld : letter tld_rest ;
tld_rest : tld_char tld_rest | ;
tld_char : letter | digit ;

/* Character sets */
letter : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' 
       | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' 
       | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'
       | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' 
       | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' 
       | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' ;

digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""
    
    test_cases = [
        ("user@example.com", True),          # Valid
        ("user.name@domain.co", True),       # Valid (simplified - allows .co)
        ("user+tag@gmail.com", True),        # Valid
        ("user_123@test-site.org", True),    # Valid
        ("a@b.co", True),                    # Valid: minimal
        ("user@", False),                    # Invalid: no domain
        ("@domain.com", False),              # Invalid: no local part
        ("user@domain", False),              # Invalid: no TLD separator
        # Note: "user@.com" would fail in the grammar because domain must start with letter/digit
        ("user@.com", False),                # Invalid: domain starts with dot
    ]
    
    print("Grammar: Email address structure\n")
    validate_with_grammar(email_grammar, test_cases)


# =============================================================================
# URL VALIDATION  
# =============================================================================

def url_regex_validation():
    """Validate URLs using regex."""
    print_subsection("URLs - REGEX Approach")
    
    # Simplified URL regex (http/https only)
    url_regex = r"^https?://[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(/[a-zA-Z0-9._~:/?#\[\]@!$&'()*+,;=-]*)?$"
    
    test_cases = [
        ("http://example.com", True),
        ("https://www.example.com", True),
        ("https://example.com/path", True),
        ("http://sub.domain.com/path/to/page", True),
        ("https://example.com/", True),
        ("ftp://example.com", False),        # Invalid: not http/https
        ("http://", False),                  # Invalid: no domain
        ("example.com", False),              # Invalid: no protocol
        ("http:/example.com", False),        # Invalid: malformed protocol
    ]
    
    print(f"Pattern: {url_regex}\n")
    for test_str, expected in test_cases:
        match = re.match(url_regex, test_str)
        result = "VALID" if match else "INVALID"
        status = "✓" if (match is not None) == expected else "✗"
        print(f"  {status} {test_str:<40} → {result}")


def url_yaep_validation():
    """Validate URLs using YAEP grammar."""
    print_subsection("URLs - YAEP Grammar Approach")
    
    url_grammar = """
TERM;

/* URL structure: protocol://domain[/path] */
url : protocol ':' '/' '/' domain path_opt ;

/* Protocol: http or https */
protocol : 'h' 't' 't' 'p' 's'
         | 'h' 't' 't' 'p' ;

/* Domain with TLD */
domain : domain_part '.' tld ;

domain_part : domain_char domain_part_rest ;
domain_part_rest : domain_char domain_part_rest 
                 | '.' domain_char domain_part_rest
                 | ;

domain_char : letter | digit | '-' ;

/* TLD */
tld : letter letter_rest ;
letter_rest : letter letter_rest | ;

/* Optional path */
path_opt : '/' path_chars | ;
path_chars : path_char path_chars | ;
path_char : letter | digit | '/' | '.' | '_' | '-' | '~' | ':' | '?' 
          | '#' | '@' | '!' | '$' | '&' | '(' | ')' | '*' | '+' 
          | ',' | ';' | '=' ;

/* Character sets */
letter : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' 
       | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' 
       | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'
       | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' 
       | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' 
       | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' ;

digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""
    
    test_cases = [
        ("http://example.com", True),
        ("https://www.example.com", True),
        ("https://example.com/path", True),
        ("http://sub.domain.com/path/to/page", True),
        ("https://example.com/", True),
        ("ftp://example.com", False),        # Invalid: not http/https
        ("http://", False),                  # Invalid: no domain
        ("example.com", False),              # Invalid: no protocol
        ("http:/example.com", False),        # Invalid: malformed protocol
    ]
    
    print("Grammar: HTTP/HTTPS URL structure\n")
    validate_with_grammar(url_grammar, test_cases)


# =============================================================================
# IPv4 ADDRESS VALIDATION
# =============================================================================

def ipv4_regex_validation():
    """Validate IPv4 addresses using regex."""
    print_subsection("IPv4 Addresses - REGEX Approach")
    
    # Simplified IPv4 regex (doesn't validate ranges 0-255)
    ipv4_regex = r"^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$"
    
    test_cases = [
        ("192.168.1.1", True),
        ("10.0.0.1", True),
        ("255.255.255.255", True),
        ("0.0.0.0", True),
        ("8.8.8.8", True),
        ("192.168.1", False),        # Invalid: missing octet
        ("192.168.1.1.1", False),    # Invalid: too many octets
        ("192.168.1.a", False),      # Invalid: non-digit
        ("999.999.999.999", True),   # Note: Regex allows (doesn't check range)
    ]
    
    print(f"Pattern: {ipv4_regex}")
    print("Note: This regex doesn't validate that octets are 0-255\n")
    for test_str, expected in test_cases:
        match = re.match(ipv4_regex, test_str)
        result = "VALID" if match else "INVALID"
        status = "✓" if (match is not None) == expected else "✗"
        print(f"  {status} {test_str:<25} → {result}")


def ipv4_yaep_validation():
    """Validate IPv4 addresses using YAEP grammar."""
    print_subsection("IPv4 Addresses - YAEP Grammar Approach")
    
    ipv4_grammar = """
TERM;

/* IPv4 address: four octets separated by dots */
ipv4 : octet '.' octet '.' octet '.' octet ;

/* Octet: 1-3 digits (doesn't validate 0-255 range) */
octet : digit 
      | digit digit 
      | digit digit digit ;

/* Digits */
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""
    
    test_cases = [
        ("192.168.1.1", True),
        ("10.0.0.1", True),
        ("255.255.255.255", True),
        ("0.0.0.0", True),
        ("8.8.8.8", True),
        ("192.168.1", False),        # Invalid: missing octet
        ("192.168.1.1.1", False),    # Invalid: too many octets
        ("192.168.1.a", False),      # Invalid: non-digit
        ("999.999.999.999", True),   # Note: Grammar allows (doesn't check range)
    ]
    
    print("Grammar: IPv4 address structure")
    print("Note: This grammar doesn't validate that octets are 0-255\n")
    validate_with_grammar(ipv4_grammar, test_cases)


# =============================================================================
# CREDIT CARD NUMBER VALIDATION (simplified)
# =============================================================================

def credit_card_regex_validation():
    """Validate credit card numbers using regex."""
    print_subsection("Credit Card Numbers - REGEX Approach")
    
    # Simplified credit card regex (16 digits with optional spaces/dashes)
    cc_regex = r"^\d{4}[\s-]?\d{4}[\s-]?\d{4}[\s-]?\d{4}$"
    
    test_cases = [
        ("1234567890123456", True),
        ("1234-5678-9012-3456", True),
        ("1234 5678 9012 3456", True),
        ("1234567890123", False),        # Invalid: too few digits
        ("12345678901234567", False),    # Invalid: too many digits
        ("1234-5678-9012-345", False),   # Invalid: last group too short
    ]
    
    print(f"Pattern: {cc_regex}")
    print("Note: This only validates format, not actual validity (no Luhn)\n")
    for test_str, expected in test_cases:
        match = re.match(cc_regex, test_str)
        result = "VALID" if match else "INVALID"
        status = "✓" if (match is not None) == expected else "✗"
        print(f"  {status} {test_str:<25} → {result}")


def credit_card_yaep_validation():
    """Validate credit card numbers using YAEP grammar."""
    print_subsection("Credit Card Numbers - YAEP Grammar Approach")
    
    cc_grammar = """
TERM;

/* Credit card: 16 digits in 4 groups */
card : group sep1 group sep2 group sep3 group ;

/* Each group is 4 digits */
group : digit digit digit digit ;

/* Separator: space, dash, or nothing */
sep1 : ' ' | '-' | ;
sep2 : ' ' | '-' | ;
sep3 : ' ' | '-' | ;

/* Digits */
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
"""
    
    test_cases = [
        ("1234567890123456", True),
        ("1234-5678-9012-3456", True),
        ("1234 5678 9012 3456", True),
        ("1234567890123", False),        # Invalid: too few digits
        ("12345678901234567", False),    # Invalid: too many digits
        ("1234-5678-9012-345", False),   # Invalid: last group too short
    ]
    
    print("Grammar: Credit card number structure")
    print("Note: This only validates format, not actual validity (no Luhn)\n")
    validate_with_grammar(cc_grammar, test_cases)


# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def validate_with_grammar(grammar_text, test_cases):
    """
    Helper function to validate strings using a YAEP grammar.
    
    Args:
        grammar_text: The YAEP grammar definition
        test_cases: List of (test_string, expected_result) tuples
    """
    try:
        grammar = Grammar()
        rc = grammar.parse_description(grammar_text)
        if rc != 0:
            print(f"  ✗ ERROR: Failed to parse grammar: {grammar.error_message()}")
            grammar.free()
            return
        
        for test_str, expected in test_cases:
            # Convert string to token stream (each char as its Unicode code point)
            tokens = [ord(c) for c in test_str]
            
            # Parse the token stream
            rc, tree, syntax_err = grammar.parse(tokens)
            
            # Determine if parse was successful
            # A successful parse means rc=0, tree exists, AND no syntax error
            parsed = (rc == 0 and tree is not None and not syntax_err)
            result = "VALID" if parsed else "INVALID"
            
            # Check if result matches expected
            status = "✓" if parsed == expected else "✗"
            
            # Display result
            width = 40 if 'url' in test_str or len(test_str) > 30 else 30
            print(f"  {status} {test_str:<{width}} → {result}")
            
            # Clean up tree if it was created
            if tree is not None:
                tree.free()
        
        grammar.free()
        
    except Exception as e:
        print(f"  ✗ ERROR: {e}")


# =============================================================================
# MAIN
# =============================================================================

def main():
    """Main demonstration function."""
    print_section("YAEP Grammar vs Regex: String Format Validation")
    
    print("""
This example demonstrates how YAEP grammars can be used as an alternative
to regular expressions for validating string formats. We compare both
approaches across several common validation tasks.

Key Differences:
- REGEX: Fast, concise patterns; limited structure extraction
- YAEP:  Formal grammar parsing; can extract structure and relationships;
         provides detailed parse trees; better for complex formats

Both approaches accept or reject the same test inputs.
""")
    
    # Run all validation examples
    print_section("1. Phone Number Validation")
    phone_regex_validation()
    phone_yaep_validation()
    
    print_section("2. Email Address Validation")
    email_regex_validation()
    email_yaep_validation()
    
    print_section("3. URL Validation")
    url_regex_validation()
    url_yaep_validation()
    
    print_section("4. IPv4 Address Validation")
    ipv4_regex_validation()
    ipv4_yaep_validation()
    
    print_section("5. Credit Card Number Validation")
    credit_card_regex_validation()
    credit_card_yaep_validation()
    
    # Summary
    print_section("Summary")
    print("""
When to use REGEX:
  • Simple pattern matching and validation
  • Performance is critical
  • No need for parse tree or structure extraction
  • Quick prototyping

When to use YAEP Grammar:
  • Complex nested structures
  • Need to extract/transform parsed data
  • Want formal grammar specification
  • Need detailed error reporting
  • Building interpreters or compilers
  
Both are valid tools - choose based on your specific requirements!
""")


if __name__ == "__main__":
    sys.exit(main())
