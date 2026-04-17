import os
import re

from sphinx.transforms import SphinxTransform
from sphinx import addnodes
from docutils import nodes

class AutoIndexSectionTransform(SphinxTransform):
    # Priority 100 runs early during the parsing phase
    default_priority = 100

    def apply(self, **kwargs):
        for section in self.document.findall(nodes.section):
            # Check for existing index node
            if any(isinstance(child, addnodes.index) for child in section):
                continue

            # Check if the section actually has a title node at index 0
            if not section.children or not isinstance(section[0], nodes.title):
                continue

            if section['names']:
                entry_text = section['names'][0]
            else:
                entry_text = section[0].astext() if section.children else "Unnamed Section"
            entry_text = entry_text.split('\n')[0]
            entry_text = entry_text.strip(":. \n\t\r")

            clean_title = re.sub(r'^(?:step )?(?:STEP )?[0-9]+[a-zA-Z]*(?:\.[0-9]+)*[:.]*\s+', '', entry_text, flags=re.IGNORECASE)
            clean_title = re.sub(r'^[ABab]\. ', '', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'2[- ]?d', '2-D', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'3[- ]?d', '3-D', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'2[- ]?dsa', '2DSA', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'2[- ]?Dimensional', '2-Dimensional', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'3[- ]?Dimensional', '3-Dimensional', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'Dmga', 'DMGA', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'pcsa', 'PCSA', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'openauc', 'OpenAUC', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'lims', 'LIMS', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'Rmsd', 'RMSD', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'UltraScan', 'UltraScan', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'UltraScan-?iii', 'UltraScan-III', clean_title, flags=re.IGNORECASE)
            clean_title = re.sub(r'van\s*Holde\s*-\s*Weischet', 'van Holde-Weischet', clean_title, flags=re.IGNORECASE)
            clean_title = clean_title.strip()
            if clean_title[0].lower() == clean_title[0]:
                clean_title = clean_title[0].upper() + clean_title[1:]
            if not section['ids']:
                section['ids'].append(nodes.make_id(clean_title))

            target_id = section['ids'][0]
            index_entry = ('single', clean_title, target_id, '', None)

            index_node = addnodes.index(entries=[index_entry])
            section.insert(1, index_node)

def setup(app):
    app.add_transform(AutoIndexSectionTransform)

    return {
        'version': '7.0',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }