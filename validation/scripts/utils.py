import os
import sys
import re
import subprocess
import random
import markdown
from string import Template
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.charset import add_charset, QP

import validationpath


def get_greeting(name):
    """
    Get a random greeting and closing statement using name
    """
    greetings = ["Dear", "Dearest", "Hi", "Salutations", "Hello", "Exalted", "Yo",
                 "Honorable", "To the esteemed", "Magnificent", "Glorious", "Howdy"]
    closing = ["Bye", "Cheers", "Best Regards", "Best Wishes", "Yours sincerely",
               "Yours truly", "Thank you", "Seeya", "Toodle-loo", "Ciao", "Hochachtungsvoll"]
    return {"greeting": "%s %s" % (random.choice(greetings), name), "closing": random.choice(closing)}


def markdown_to_plaintext(text):
    """
    removes markdown specific formatting elements from text
    """
    # replace all links of the form [text](url) by just the url.
    replace_links = re.compile(r"\[.*?]\(([^)]*)\)")
    text = replace_links.sub(r"\1", text)
    # also replace all {: ... } braces which contain attribute lists
    replace_attrs = re.compile(r'\{:([^\}\n]*?)\}')
    text = replace_attrs.sub(r"", text)
    # replace <http://link>, <https://link> and <me@mail.com> by removing <>
    replace_autolink = re.compile(r"<(https?://[^>]*|[^> ]+@[^> ]+)>")
    text = replace_autolink.sub(r"\1", text)
    return text


def send_mail(name, recipient, subject, text, link=None, link_title=None, mood="normal"):
    """
    Send an email to `name` at mail address `recipient` with the given subject and text.

    This function will add a greeting at the top and a closing statement at the
    bottom and convert the mail text to html using markdown.

    The optional link/link_title argument can be used to easily add a link to
    the mail for users to click.
    """
    all_moods = ["happy", "normal", "meh", "angry", "livid", "dead"]
    if mood not in all_moods:
        raise KeyError("Unknown mood please use one of {}".format(", ".join(all_moods)))

    add_charset("utf-8", QP, QP, "utf-8")
    msg = MIMEMultipart('alternative')
    msg['Subject'] = subject
    msg['From'] = "B2Bot <b2soft@mail.desy.de>"
    msg['Reply-To'] = "Martin Ritter <martin.ritter@belle2.org>"
    if "bamboo_email_override" in os.environ and os.environ["bamboo_email_override"].find("@") > 0:
        msg["To"] = os.environ["bamboo_email_override"]
    else:
        msg['To'] = recipient

    if "bamboo_email_bcc" in os.environ and os.environ["bamboo_email_bcc"].find("@") > 0:
        msg['Bcc'] = os.environ["bamboo_email_bcc"]

    data = get_greeting(name)
    data["mood"] = mood
    data["title"] = subject
    data["text"] = markdown_to_plaintext(text)
    data["plain_link"] = ""
    # Format the final goto link correctly if it is given
    if link is not None:
        if link_title is not None:
            text += "\n\n[{}]({}){{:.goto}}".format(link_title, link)
            data["plain_link"] = "\n\n{}: {}".format(link_title, link)
        else:
            text += "\n\n<{}>{{:.goto}}".format(link)
            data["plain_link"] = "\n\n{}".format(link)

    # parse plain text to html
    data["body"] = markdown.markdown(text, output_format="xhtml1", extensions=["markdown.extensions.attr_list"])
    # create multipart mime mail
    msg.attach(MIMEText("{greeting},\n\n{text}{plain_link}\n\n{closing}"
                        "\n\tThe Belle 2 Software Bot (B2Bot)".format(**data), "plain"))
    template = Template(
        open(
            os.path.join(
                validationpath.get_basepath()["local"],
                "validation/html_static/templates/template_mail.html")).read())
    msg.attach(MIMEText(template.substitute(**data), "html"))

    if os.environ.get("bamboo_DRYRUN", False):
        # print("Send Mail: ", msg.as_bytes().decode(), file=sys.stderr)
        open(msg["To"] + ".html", "w").write(template.substitute(**data))
    else:
        subprocess.run(["/usr/sbin/sendmail", "-t", "-oi"], input=msg.as_bytes())
