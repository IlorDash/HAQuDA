import contextlib
import io
import os
import sys
import tempfile
import unittest
import zlib
from unittest import mock

import gen_web_pages as gen


class JsStringsTest(unittest.TestCase):
    def test_escaped_single_quote_does_not_end_string(self):
        src = "a = 'it\\'s // x'; // c\nb"
        exp_text = "a = 'it\\'s // x';\nb"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_apostrophe_inside_line_comment_does_not_start_string(self):
        src = "a = 1; // the lamp's refusal\nb = '…Ж';\n"
        exp_text = "a = 1;\nb = '…Ж';\n"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_escaped_double_quote_does_not_end_string(self):
        src = 'a = "say \\"hi\\" /* x */"; // c\nb'
        exp_text = 'a = "say \\"hi\\" /* x */";\nb'

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_escaped_backslash_ends_string_before_real_comment(self):
        src = 'a = "a\\\\"; // c'
        exp_text = 'a = "a\\\\";'

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_line_comment_marker_inside_string_is_kept(self):
        src = "s = 'http://x'; // c"
        exp_text = "s = 'http://x';"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_block_comment_marker_inside_string_is_kept(self):
        src = 's = "/* x */"; // c'
        exp_text = 's = "/* x */";'

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)


class JsTemplatesTest(unittest.TestCase):
    def test_comment_markers_inside_template_are_kept(self):
        src = "t = `// a /* b */`; // c"
        exp_text = "t = `// a /* b */`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_comment_inside_substitution_is_removed(self):
        src = "t = `a${ x // c\n}b`;"
        exp_text = "t = `a${ x\n}b`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_nested_template_with_strings_and_comments(self):
        src = "t = `a${ f('//', `b${ /* c */x }`) }d`; // e"
        exp_text = "t = `a${ f('//', `b${  x }`) }d`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_closing_brace_inside_string_in_substitution_does_not_end_it(self):
        src = "t = `${ '}' /* c */ }//not a comment`; // d"
        exp_text = "t = `${ '}'   }//not a comment`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_escaped_backtick_does_not_end_template(self):
        src = "t = `a\\`// b`; // c"
        exp_text = "t = `a\\`// b`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_object_braces_inside_substitution_keep_template_open(self):
        src = "t = `${ {a: 1}.a }// x`; // c"
        exp_text = "t = `${ {a: 1}.a }// x`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)


class JsRegexTest(unittest.TestCase):
    def test_escaped_slashes_inside_regex_are_kept(self):
        src = "r = /a\\/\\/b/; // c"
        exp_text = "r = /a\\/\\/b/;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_slash_inside_class_does_not_end_regex(self):
        src = "r = /[/]\\//; // c"
        exp_text = "r = /[/]\\//;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_escaped_star_after_slash_is_not_a_comment(self):
        src = "r = /\\/\\*/g; // c"
        exp_text = "r = /\\/\\*/g;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_quote_inside_regex_does_not_start_string(self):
        src = "r = /\"/.test(s); // c"
        exp_text = "r = /\"/.test(s);"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_regex_after_open_parenthesis(self):
        src = "f(/a'b/); // c"
        exp_text = "f(/a'b/);"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_regex_after_comma(self):
        src = "f(a, /a'b/); // c"
        exp_text = "f(a, /a'b/);"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_regex_after_colon(self):
        src = "o = {k: /a'b/}; // c"
        exp_text = "o = {k: /a'b/};"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_regex_after_return(self):
        src = "return /a'b/.test(s); // c"
        exp_text = "return /a'b/.test(s);"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_regex_after_closing_brace_is_read_as_division_known_limit(self):
        src = "{}\n/a'b/.test(s);"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")


class JsDivisionTest(unittest.TestCase):
    def test_division_after_closing_parenthesis(self):
        src = "x = (a) / 2; /* c */\ny"
        exp_text = "x = (a) / 2;\ny"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_closing_bracket(self):
        src = "x = a[1] / 2; /* c */\ny"
        exp_text = "x = a[1] / 2;\ny"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_closing_brace(self):
        src = "x = {a: 1}.a / 2; // c\ny = ({}) / 2; /* c */\nz"
        exp_text = "x = {a: 1}.a / 2;\ny = ({}) / 2;\nz"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_identifier(self):
        src = "x = a / b; // c\ny"
        exp_text = "x = a / b;\ny"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_number(self):
        src = "x = 1 / 2; // c\ny"
        exp_text = "x = 1 / 2;\ny"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_postfix_increment(self):
        src = "a=i++ / 2; /* x */ b=1 / 2"
        exp_text = "a=i++ / 2;   b=1 / 2"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_postfix_decrement(self):
        src = "x = i-- / 2 // c"
        exp_text = "x = i-- / 2"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_division_after_increment_followed_by_plus(self):
        src = "a+++b / 2 /*c*/"
        exp_text = "a+++b / 2"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)


class CommentLayoutTest(unittest.TestCase):
    def test_multi_line_block_comment_keeps_one_newline(self):
        src = "a /* x\ny */ b"
        exp_text = "a\n b"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_empty_block_comment_between_words_becomes_space(self):
        src = "a/**/b"
        exp_text = "a b"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_line_left_blank_by_removed_comment_is_dropped(self):
        src = "a\n// c\nb"
        exp_text = "a\nb"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_real_blank_lines_are_kept(self):
        src = "a\n\n// c\n\nb"
        exp_text = "a\n\n\nb"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_line_comment_at_end_without_newline(self):
        src = "a = 1; // c"
        exp_text = "a = 1;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_block_comment_at_end_without_newline(self):
        src = "a = 1; /* c */"
        exp_text = "a = 1;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)

    def test_text_without_comments_is_unchanged(self):
        src = "a = 1;\n\n  b = 'x';\n"
        exp_text = src

        res = gen.strip_js(src, "t.js")

        self.assertEqual(exp_text, res)


class CssTest(unittest.TestCase):
    def test_block_comment_is_removed(self):
        src = "a { b: c; } /* x */\nd {}"
        exp_text = "a { b: c; }\nd {}"

        res = gen.strip_css(src, "t.css")

        self.assertEqual(exp_text, res)

    def test_comment_marker_inside_double_quoted_string_is_kept(self):
        src = 'a { content: "/* x */"; } /* c */'
        exp_text = 'a { content: "/* x */"; }'

        res = gen.strip_css(src, "t.css")

        self.assertEqual(exp_text, res)

    def test_comment_marker_inside_quoted_url_is_kept(self):
        src = "a { background: url('/*x*/.png'); } /* c */"
        exp_text = "a { background: url('/*x*/.png'); }"

        res = gen.strip_css(src, "t.css")

        self.assertEqual(exp_text, res)

    def test_line_comment_marker_is_not_a_css_comment(self):
        src = "a { b: c; } // not a comment"
        exp_text = src

        res = gen.strip_css(src, "t.css")

        self.assertEqual(exp_text, res)

    def test_strip_comments_picks_css_rules_by_extension(self):
        src = "a { b: c; } /* x */"
        exp_text = "a { b: c; }"

        res = gen.strip_comments("style.css", src)

        self.assertEqual(exp_text, res)

    def test_strip_comments_picks_js_rules_by_extension(self):
        src = "a = 1; // x"
        exp_text = "a = 1;"

        res = gen.strip_comments("app.js", src)

        self.assertEqual(exp_text, res)


class HtmlTest(unittest.TestCase):
    def test_html_comment_is_removed(self):
        src = "<p>a</p><!-- c -->\n<p>b</p>"
        exp_text = "<p>a</p>\n<p>b</p>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_multi_line_html_comment_is_removed(self):
        src = "<p>a</p>\n<!-- c\nd -->\n<p>b</p>"
        exp_text = "<p>a</p>\n<p>b</p>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_comment_inside_pre_is_kept(self):
        src = "<pre><!-- keep --></pre><!-- c -->"
        exp_text = "<pre><!-- keep --></pre>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_comment_inside_textarea_is_kept(self):
        src = "<textarea><!-- keep --></textarea><!-- c -->"
        exp_text = "<textarea><!-- keep --></textarea>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_html_comment_marker_inside_script_string_is_kept(self):
        src = '<script>var s = "<!-- k -->"; // c\n</script>'
        exp_text = '<script>var s = "<!-- k -->";\n</script>'

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_inline_script_is_stripped_with_js_rules(self):
        src = "<script>var a = 1; // c\nvar b = /* d */ 2;</script>"
        exp_text = "<script>var a = 1;\nvar b =   2;</script>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_inline_style_is_stripped_with_css_rules(self):
        src = "<style>a { b: c; } /* x */\n</style>"
        exp_text = "<style>a { b: c; }\n</style>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_script_with_src_is_left_untouched(self):
        src = '<script src="/app.js">// kept\n</script>'
        exp_text = src

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)

    def test_script_tag_is_found_case_insensitively(self):
        src = "<SCRIPT>a = 1; // c\n</SCRIPT>"
        exp_text = "<SCRIPT>a = 1;\n</SCRIPT>"

        res = gen.strip_comments("t.html", src)

        self.assertEqual(exp_text, res)


class StripErrorTest(unittest.TestCase):
    def test_js_unterminated_single_quoted_string(self):
        src = "a = 'abc\nb"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_js_unterminated_double_quoted_string_at_end(self):
        src = 'a = "abc'

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_js_unterminated_template(self):
        src = "a = `abc"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_js_unterminated_regex(self):
        src = "a = /abc\nb"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_js_unterminated_block_comment(self):
        src = "a = 1; /* abc"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_css_unterminated_block_comment(self):
        src = "a { b: c; } /* abc"

        with self.assertRaises(gen.StripError):
            gen.strip_css(src, "t.css")

    def test_css_unterminated_string(self):
        src = 'a { content: "abc; }'

        with self.assertRaises(gen.StripError):
            gen.strip_css(src, "t.css")

    def test_html_unterminated_comment(self):
        src = "<p>a</p><!-- abc"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_html_unclosed_script(self):
        src = "<script>a = 1;"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_html_unclosed_style(self):
        src = "<style>a { b: c; }"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_html_unclosed_pre(self):
        src = "<pre>abc"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_html_unclosed_textarea(self):
        src = "<textarea>abc"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_html_inline_script_error_is_raised(self):
        src = "<script>a = 'abc\n</script>"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_js_unterminated_template_substitution(self):
        src = "`${ b"

        with self.assertRaises(gen.StripError):
            gen.strip_js(src, "t.js")

    def test_js_unterminated_template_substitution_names_its_line(self):
        src = "x\n`a${ b"

        with self.assertRaises(gen.StripError) as ctx:
            gen.strip_js(src, "t.js")

        self.assertIn("near line 2", str(ctx.exception))

    def test_html_inline_script_unterminated_template_substitution(self):
        src = "<script>a = `${ b</script>"

        with self.assertRaises(gen.StripError):
            gen.strip_html(src, "t.html")

    def test_js_closed_nested_template_substitution_is_kept(self):
        src = "x = `a${ `b${c}` }d`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(src, res)

    def test_js_object_literal_inside_template_substitution_is_kept(self):
        src = "x = `a${ {k: 1}.k }d`;"

        res = gen.strip_js(src, "t.js")

        self.assertEqual(src, res)


class MainTest(unittest.TestCase):
    def test_main_returns_one_and_names_the_error_when_stripping_fails(self):
        exp_code = 1
        exp_message = "cannot strip comments"
        stderr = io.StringIO()

        with mock.patch.object(sys, "argv", ["gen_web_pages.py"]), \
                mock.patch.object(gen, "read_source", return_value=b"<!-- open"), \
                contextlib.redirect_stderr(stderr):
            res = gen.main()

        self.assertEqual(exp_code, res)
        self.assertIn(exp_message, stderr.getvalue())

    def test_main_check_returns_one_when_stripping_fails(self):
        exp_code = 1
        stderr = io.StringIO()

        with mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                mock.patch.object(gen, "SOURCES", ["app.js"]), \
                mock.patch.object(gen, "read_source", return_value=b"a = 'open"), \
                contextlib.redirect_stderr(stderr):
            res = gen.main()

        self.assertEqual(exp_code, res)


class HeaderOutputTest(unittest.TestCase):
    def test_main_writes_the_rendered_header_and_returns_zero(self):
        exp_code = 0
        exp_message = "wrote "
        page = b"var a = 1; // note\n"
        stdout = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with mock.patch.object(sys, "argv", ["gen_web_pages.py"]), \
                    mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page), \
                    contextlib.redirect_stdout(stdout):
                exp_bytes = gen.render()

                res = gen.main()

            with open(output, "rb") as handle:
                written = handle.read()

        self.assertEqual(exp_code, res)
        self.assertEqual(exp_bytes, written)
        self.assertIn(exp_message, stdout.getvalue())

    def test_rendered_header_holds_one_gzip_array_per_source(self):
        exp_array = b"const uint8_t WEB_APP_JS_GZ[] PROGMEM = {"
        exp_length = b"constexpr size_t WEB_APP_JS_GZ_LEN = sizeof(WEB_APP_JS_GZ);"
        page = b"var a = 1;\n"

        with mock.patch.object(gen, "SOURCES", ["app.js"]), \
                mock.patch.object(gen, "read_source", return_value=page):
            res = gen.render()

        self.assertIn(exp_array, res)
        self.assertIn(exp_length, res)

    def test_check_returns_zero_for_the_header_main_wrote(self):
        exp_code = 0
        exp_message = "up to date"
        page = b"var a = 1;\n"
        stdout = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page):
                with open(output, "wb") as handle:
                    handle.write(gen.render())
                with mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                        contextlib.redirect_stdout(stdout):
                    res = gen.main()

        self.assertEqual(exp_code, res)
        self.assertIn(exp_message, stdout.getvalue())

    def test_check_accepts_a_header_with_crlf_line_endings(self):
        exp_code = 0
        exp_message = "up to date"
        page = b"var a = 1;\n"
        stdout = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page):
                with open(output, "wb") as handle:
                    handle.write(gen.render().replace(b"\n", b"\r\n"))
                with mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                        contextlib.redirect_stdout(stdout):
                    res = gen.main()

        self.assertEqual(exp_code, res)
        self.assertIn(exp_message, stdout.getvalue())

    def test_check_returns_one_for_a_header_of_other_pages(self):
        exp_code = 1
        exp_message = "stale"
        old_page = b"var a = 1;\n"
        new_page = b"var a = 2;\n"
        stderr = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]):
                with mock.patch.object(gen, "read_source", return_value=old_page):
                    with open(output, "wb") as handle:
                        handle.write(gen.render())
                with mock.patch.object(gen, "read_source", return_value=new_page), \
                        mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                        contextlib.redirect_stderr(stderr):
                    res = gen.main()

        self.assertEqual(exp_code, res)
        self.assertIn(exp_message, stderr.getvalue())

    def test_check_returns_one_for_a_header_that_is_not_arrays(self):
        exp_code = 1
        page = b"var a = 1;\n"
        stderr = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with open(output, "wb") as handle:
                handle.write(b"#pragma once\n// not a generated header\n")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page), \
                    mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                    contextlib.redirect_stderr(stderr):
                res = gen.main()

        self.assertEqual(exp_code, res)

    def test_check_returns_one_when_the_header_is_missing(self):
        exp_code = 1
        page = b"var a = 1;\n"
        stderr = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "missing", "WebPages.h")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page), \
                    mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                    contextlib.redirect_stderr(stderr):
                res = gen.main()

        self.assertEqual(exp_code, res)

    def test_check_accepts_the_current_pages_with_other_deflate_bytes(self):
        exp_code = 0
        exp_message = "different zlib build"
        page = b"var a = 1; var b = 2; var c = 3; var d = 4;\n"
        stored_level = 0
        stdout = io.StringIO()

        with tempfile.TemporaryDirectory() as directory:
            output = os.path.join(directory, "WebPages.h")
            with mock.patch.object(gen, "OUTPUT", output), \
                    mock.patch.object(gen, "SOURCES", ["app.js"]), \
                    mock.patch.object(gen, "read_source", return_value=page):
                exp_bytes = gen.render()
                stored = zlib.compressobj(stored_level, zlib.DEFLATED, -15)
                with mock.patch.object(gen.zlib, "compressobj", return_value=stored):
                    other_bytes = gen.render()
                with open(output, "wb") as handle:
                    handle.write(other_bytes)
                with mock.patch.object(sys, "argv", ["gen_web_pages.py", "--check"]), \
                        contextlib.redirect_stdout(stdout):
                    res = gen.main()

        self.assertNotEqual(exp_bytes, other_bytes)
        self.assertEqual(exp_code, res)
        self.assertIn(exp_message, stdout.getvalue())


class RealSourcesTest(unittest.TestCase):
    def test_every_source_strips_without_error(self):
        for name in gen.SOURCES:
            with self.subTest(source=name):
                text = gen.read_source(name).decode("utf-8")

                res = gen.strip_comments(name, text)

                self.assertTrue(res)

    def test_stripping_twice_gives_the_same_text_as_once(self):
        for name in gen.SOURCES:
            with self.subTest(source=name):
                once = gen.strip_comments(name, gen.read_source(name).decode("utf-8"))

                res = gen.strip_comments(name, once)

                self.assertEqual(once, res)

    def test_served_js_and_css_equal_stripped_source(self):
        for name in ("app.js", "wifi.js", "buttons.js", "style.css"):
            with self.subTest(source=name):
                exp_bytes = gen.strip_comments(name, gen.read_source(name).decode("utf-8")).encode("utf-8")

                res = gen.served(name)

                self.assertEqual(exp_bytes, res)

    def test_served_pages_reference_every_asset_with_a_hash(self):
        for name in ("index.html", "wifi_manager.html", "buttons.html"):
            with self.subTest(source=name):
                text = gen.served(name).decode("utf-8")

                self.assertNotIn('"/style.css"', text)
                self.assertNotIn('"/app.js"', text)

    def test_buttons_page_references_its_script_with_a_hash(self):
        exp_ref = '"/buttons.js?v=' + gen.asset_hash("buttons.js") + '"'

        res = gen.served("buttons.html").decode("utf-8")

        self.assertIn(exp_ref, res)
        self.assertNotIn('"/buttons.js"', res)


class AssetHashTest(unittest.TestCase):
    def test_hash_ignores_an_added_comment(self):
        plain = b"var a = 1;\n"
        commented = b"var a = 1; // note\n/* more */\n"

        with mock.patch.object(gen, "read_source", return_value=plain):
            exp_hash = gen.asset_hash("app.js")
        with mock.patch.object(gen, "read_source", return_value=commented):
            res = gen.asset_hash("app.js")

        self.assertEqual(exp_hash, res)

    def test_hash_changes_when_code_changes(self):
        before = b"var a = 1;\n"
        after = b"var a = 2;\n"

        with mock.patch.object(gen, "read_source", return_value=before):
            old_hash = gen.asset_hash("app.js")
        with mock.patch.object(gen, "read_source", return_value=after):
            res = gen.asset_hash("app.js")

        self.assertNotEqual(old_hash, res)

    def test_page_reference_carries_the_asset_hash(self):
        pages = {
            "index.html": b'<link href="/style.css"><script src="/app.js"></script>',
            "style.css": b"a { b: c; }",
            "app.js": b"var a = 1;",
            "wifi.js": b"var b = 2;",
            "buttons.js": b"var c = 3;",
        }

        with mock.patch.object(gen, "read_source", side_effect=pages.__getitem__):
            exp_css = b'"/style.css?v=' + gen.asset_hash("style.css").encode("ascii") + b'"'
            exp_js = b'"/app.js?v=' + gen.asset_hash("app.js").encode("ascii") + b'"'
            res = gen.served("index.html")

        self.assertIn(exp_css, res)
        self.assertIn(exp_js, res)

    def test_page_is_unchanged_when_only_an_asset_comment_is_added(self):
        page = b'<script src="/app.js"></script>'
        plain = {"index.html": page, "style.css": b"", "wifi.js": b"", "buttons.js": b"", "app.js": b"var a = 1;"}
        commented = {"index.html": page, "style.css": b"", "wifi.js": b"", "buttons.js": b"", "app.js": b"var a = 1; // note"}

        with mock.patch.object(gen, "read_source", side_effect=plain.__getitem__):
            exp_page = gen.served("index.html")
        with mock.patch.object(gen, "read_source", side_effect=commented.__getitem__):
            res = gen.served("index.html")

        self.assertEqual(exp_page, res)

    def test_page_changes_when_asset_code_changes(self):
        page = b'<script src="/app.js"></script>'
        before = {"index.html": page, "style.css": b"", "wifi.js": b"", "buttons.js": b"", "app.js": b"var a = 1;"}
        after = {"index.html": page, "style.css": b"", "wifi.js": b"", "buttons.js": b"", "app.js": b"var a = 2;"}

        with mock.patch.object(gen, "read_source", side_effect=before.__getitem__):
            old_page = gen.served("index.html")
        with mock.patch.object(gen, "read_source", side_effect=after.__getitem__):
            res = gen.served("index.html")

        self.assertNotEqual(old_page, res)


class LineEndingsTest(unittest.TestCase):
    def test_crlf_source_is_served_like_lf_source(self):
        lf_text = b"var a = 1; // note\nvar b = 2; /* x\ny */\nvar c = 3;\n"
        crlf_text = lf_text.replace(b"\n", b"\r\n")

        with tempfile.TemporaryDirectory() as lf_dir, tempfile.TemporaryDirectory() as crlf_dir:
            with open(os.path.join(lf_dir, "app.js"), "wb") as handle:
                handle.write(lf_text)
            with open(os.path.join(crlf_dir, "app.js"), "wb") as handle:
                handle.write(crlf_text)
            with mock.patch.object(gen, "SCRIPT_DIR", lf_dir):
                exp_bytes = gen.served("app.js")
            with mock.patch.object(gen, "SCRIPT_DIR", crlf_dir):
                res = gen.served("app.js")

        self.assertEqual(exp_bytes, res)

    def test_crlf_source_has_no_carriage_returns_after_reading(self):
        text = b"a\r\nb\r\n"
        exp_bytes = b"a\nb\n"

        with tempfile.TemporaryDirectory() as directory:
            with open(os.path.join(directory, "app.js"), "wb") as handle:
                handle.write(text)
            with mock.patch.object(gen, "SCRIPT_DIR", directory):
                res = gen.read_source("app.js")

        self.assertEqual(exp_bytes, res)


if __name__ == "__main__":
    unittest.main()
