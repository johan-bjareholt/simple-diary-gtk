#!/usr/bin/env python3

import sys
import os
import shutil

import marko

USAGE = f"{sys.argv[0]} md-dir tex-dir"

if len(sys.argv) < 3:
    print("Not enough arguments")
    print(USAGE)
    exit(1)

md_dir = sys.argv[1]
tex_dir = sys.argv[2]

try:
    os.makedirs(tex_dir)
except FileExistsError:
    print(f"folder already exists: {tex_dir}")
    exit(1)

def filepath_escape(string):
    #string = string.replace("\"", "\\\"")
    return string

def latex_escape(string):
    # Following chars have special meaning in latex:
    #   & % $ # _ { } ~ ^ \
    string = string.replace("\\", "\\\\")
    string = string.replace("\"", "''")
    # TODO: Also do single quotes
    string = string.replace("%", "\%")
    string = string.replace("$", "\$")
    string = string.replace("#", "\#")
    string = string.replace("_", "\_")
    string = string.replace("{", "\{")
    string = string.replace("}", "\}")
    string = string.replace("&", "\\&")
    string = string.replace("~", "\~")
    string = string.replace("^", "\^")
    return string


class Image:
    def __init__(self, path, title):
        self.path = path
        self.title = title

class LatexRenderer(marko.renderer.Renderer):
    """The most common renderer for markdown parser"""
    def __init__(self):
        self.images = []
        marko.renderer.Renderer.__init__(self)

    def render(self, element):  # type: (Element) -> str
        # Store the root node to provide some context to render functions
        if not self.root_node:
            self.root_node = element  # type: ignore
        if hasattr(element, "get_type"):
            func_name = "render_" + element.get_type(snake_case=True)
            render_func = getattr(self, func_name, None)
            if render_func is not None and (
                getattr(render_func, "_force_delegate", False) or self.delegate
            ):
                return render_func(element)
        doc_str = self.render_children(element)

        doc_str += "\FloatBarrier\n"

        if len(self.images) > 0:
            for index, img in enumerate(self.images):
                if index % 2 == 0:
                    doc_str += "\\begin{figure}[ht]\n"
                # set width and height limits for photos
                if index == len(self.images)-1 and index % 2 == 0:
                    width = "\\textwidth"
                    height = "0.45\\textheight"
                else:
                    width = "0.5\\textwidth"
                    height = "0.45\\textheight"
                # create photos folder if it does not already exist
                img_dir = os.path.dirname(img.path)
                try:
                    os.makedirs(tex_dir + "/" + img_dir)
                except FileExistsError:
                    pass
                # copy image file
                orig_file = f"{md_dir}/{img.path}"
                dest_file = f"{tex_dir}/{img.path}"
                shutil.copy2(orig_file, dest_file)
                # append tex
                doc_str += "  \\begin{subfigure}{" + width + "}\n"
                doc_str += "    \\hfill\n"
                doc_str += "    \includegraphics[height=" + height + ",width=\\textwidth,keepaspectratio]{" + filepath_escape(img.path) + "}\\hspace{\\fill}\n"
                doc_str += "    \subcaption{" + latex_escape(img.title) + "}\n"
                doc_str += "  \end{subfigure}\n"
                if index % 2 == 1 or index == len(self.images)-1:
                    doc_str += "\end{figure}\n"

        return doc_str

    def render_paragraph(self, element):
        children = self.render_children(element)
        if element._tight:
            return children
        else:
            return f"\n{children}\n"

    def render_list(self, element):
        if element.ordered:
            tag = "ol"
            extra = f' start="{element.start}"' if element.start != 1 else ""
        else:
            tag = "ul"
            extra = ""
        return "<{tag}{extra}>\n{children}</{tag}>\n".format(
            tag=tag, extra=extra, children=self.render_children(element)
        )

    def render_list_item(self, element):
        if len(element.children) == 1 and getattr(element.children[0], "_tight", False):
            sep = ""
        else:
            sep = "\n"
        return f"<li>{sep}{self.render_children(element)}</li>\n"

    def render_quote(self, element):
        return f"<blockquote>\n{self.render_children(element)}</blockquote>\n"

    def render_fenced_code(self, element):
        lang = (
            f' class="language-{element.lang}"'
            if element.lang
            else ""
        )
        return "<pre><code{}>{}</code></pre>\n".format(
            lang, html.escape(element.children[0].children)
        )

    def render_code_block(self, element):
        return self.render_fenced_code(element)

    def render_html_block(self, element):
        return element.children

    def render_thematic_break(self, element):
        return "<hr />\n"

    def render_heading(self, element):
        return "<h{level}>{children}</h{level}>\n".format(
            level=element.level, children=self.render_children(element)
        )

    def render_setext_heading(self, element):
        return self.render_heading(element)

    def render_blank_line(self, element):
        return ""

    def render_link_ref_def(self, element):
        return ""

    def render_emphasis(self, element):
        return f"<em>{self.render_children(element)}</em>"

    def render_strong_emphasis(self, element):
        return f"<strong>{self.render_children(element)}</strong>"

    def render_inline_html(self, element):
        return self.render_html_block(element)

    def render_plain_text(self, element):
        if isinstance(element.children, str):
            return element.children
        return self.render_children(element)

    def render_link(self, element):
        template = '<a href="{}"{}>{}</a>'
        title = (
            f' title="{element.title}"'
            if element.title
            else ""
        )
        url = element.dest
        body = self.render_children(element)
        return template.format(url, title, body)

    def render_auto_link(self, element):
        return self.render_link(element)

    def render_image(self, element):
        render_func = self.render
        self.render = self.render_plain_text
        body = self.render_children(element)
        self.render = render_func

        self.images.append(Image(element.dest, latex_escape(body)))
        return ""

    def render_literal(self, element):
        return self.render_raw_text(element)

    def render_raw_text(self, element):
        return latex_escape(element.children)

    def render_line_break(self, element):
        if element.soft:
            return "\n"
        return "<br />\n"

    def render_code_span(self, element):
        return element.children
        #return f"<code>{html.escape(element.children)}</code>"


def latex2md(md_path):
    title = os.path.basename(md_path)[:-3]
    print(f"converting '{title}'")
    tex_path = f"{tex_dir}/{title}.tex"
    with open(md_path, "r") as md_file:
        tex_str = ""
        tex_str += "\section*{" + latex_escape(title) + "}\n"
        md_str = md_file.read()
        converter = marko.Markdown(renderer=LatexRenderer)
        tex_str += converter.convert(md_str)
        with open(tex_path, "w") as tex_file:
            tex_file.write(tex_str)
    return title

if os.path.isdir(md_dir):
    entries = []
    md_file_paths = os.listdir(md_dir)
    for md_path in md_file_paths:
        if not md_path.endswith(".md"):
            continue
        md_path_full = f"{md_dir}/{md_path}"
        entry_name = latex2md(md_path_full)
        entries.append(entry_name)

    root_str = ""
    root_str += "\documentclass[a4paper,12pt]{article}\n"
    root_str += "\n"
    # International characters
    root_str += "\\usepackage[utf8]{inputenc}\n"
    # Font family
    root_str += "\\usepackage[T1]{fontenc}\n"
    root_str += "\\usepackage{charter}\n"
    #
    root_str += "\\usepackage{graphicx}\n"
    root_str += "\\usepackage[space]{grffile}\n"
    root_str += "\\usepackage[labelformat=empty]{caption}\n"
    root_str += "\\usepackage[labelformat=empty]{subcaption}\n"
    # 2cm margins
    root_str += "\\usepackage{geometry}\n"
    root_str += "\\geometry{margin=2cm}\n"
    # Allow more floats
    root_str += "\\extrafloats{1000}\n"
    # No newline after sections
    root_str += "\\usepackage{newclude}\n"
    # Float barriers
    root_str += "\\usepackage[section]{placeins}\n"
    root_str += "\n"
    root_str += "\\begin{document}\n"
    root_str += "\n"

    for entry_name in sorted(entries):
        #root_str += "\\include{" + entry_name + "}\n"
        root_str += "\\include*{" + entry_name + "}\n"
    root_str += "\n"
    root_str += "\end{document}\n"
    with open(f"{tex_dir}/diary.tex", "w") as diary_file:
        diary_file.write(root_str)
else:
    if not md_dir.endswith(".md"):
        print("file has to end with .md")
        exit(1)
    latex2md(md_dir)
