#!/usr/bin/python

from __future__ import print_function
import sys
import argparse as argp
import lxml.etree as etree
import lxml.objectify as objfy

class XunitReport:

    def __init__(self, xml_path, xsd_path):
        self.tree = etree.parse(xml_path)
        xsd = etree.XMLSchema(file=xsd_path)
        xsd.assertValid(self.tree)

    def translate(self, name, translator):
        root = self.tree.getroot()
        root.set('name', name)

        translator.show_prolog()
        translator.show_suite(root)
        for e in root.iter('testsuite'):
                translator.show_suite(e)


class XunitRstTranslator:
    def show_prolog(self):
        print(".. include:: <isopub.txt>\n")

    def show_suite(self, suite):
        attrs = suite.attrib

        print("\n.. csv-table:: %s suite status" % attrs['name'])
        print("\t:widths: 50, 10, 10, 10, 10")
        print("\t:header: Name, #errors, #failures, #success, " "#count\n")

        for c in suite:
            attrs = c.attrib
            if c.tag == 'testsuite':
                success = (int(attrs['tests']) - int(attrs['errors']) -
                           int(attrs['failures']) - int(attrs['skipped']))
                print("\t|check| %s, %s, %s, %s, %s" %
                      (attrs['name'], attrs['errors'], attrs['failures'],
                       str(success), attrs['tests']))
            elif c.tag == 'testcase':
                if (attrs['status']) == 'error':
                    print("\t|cross| %s, 1, 0, 0, 1" % attrs['name'])
                elif (attrs['status']) == 'failure':
                    print("\t|cross| %s, 0, 1, 0, 1" % attrs['name'])
                elif (attrs['status']) == 'success':
                    print("\t|check| %s, 0, 0, 1, 1" % attrs['name'])
                else:
                    print("\t%s, ?, ?, ?, 1" % attrs['name'])



if __name__ == '__main__':
    parse = argp.ArgumentParser(description = 'Translate xunit compliant ' +
                                              'XML file to reStructuredText')
    parse.add_argument('xsd', metavar = 'XSD_PATH',
                       type = str, nargs = 1,
                       help = 'XSD schema used to validate input XML file')
    parse.add_argument('xml', metavar = 'XML_PATH',
                       type = str, nargs = 1,
                       help = 'XML file input to convert')
    args = parse.parse_args()

    try:
        report = XunitReport(args.xml[0], args.xsd[0])
        report.translate('Karn', XunitRstTranslator())
    except IOError, e:
        print("Failed to fetch XML input: %s" % str(e), file=sys.stderr)
        sys.exit(1)
    except etree.XMLSchemaParseError, e:
        print("Invalid XML schema: %s" % str(e), file=sys.stderr)
        sys.exit(1)
    except (etree.DocumentInvalid, etree.XMLSyntaxError), e:
        print("Failed to validate XML input: %s" % str(e), file=sys.stderr)
        sys.exit(1)
