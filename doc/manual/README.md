# UltraScan III Documentation

## Table of Contents

- [Building Documentation Files](#building-documentation-files)
- [File Organization](#file-organization)
- [Documentation Style Guidelines](#documentation-style-guidelines)
- [Example Document Structure](#example-document-structure)
- [Adding New Pages to the Documentation](#adding-new-pages-to-the-documentation)
- [Testing Documentation with Qt Assistant](#testing-documentation-with-qt-assistant)

## Building Documentation Files

To build the UltraScan III documentation files for Qt Assistant, from the `ultrascan3/doc/manual` directory, simply run:

```bash
make
```

This requires that the Qt binaries are in your PATH and the Perl tool `tpage` is available.

### Installing Required Tools

To get `tpage`, check your distribution for `libtemplate-perl` or install it yourself as root:

```bash
sudo cpan -i Template::Tools
```

Reference: http://template-toolkit.org/docs/index.html

On some installations, AppConfig may also be required:

```bash
sudo cpan -i AppConfig
```

For doxygen documentation generation, a program called "GraphViz" is needed. It's available from:
http://www.graphviz.org/Download_source.php

## File Organization

### Important Note on File Structure

During the build process:
- Source `.body` files are organized in directories for ease of maintenance
- The build process flattens this structure, placing all files in a single output directory and then creates the HTML
- Image files maintain their original folder structure under the `images/` directory

Cross-referencing implications:
- When linking to other HTML files, do not include directory paths as they won't exist in the built documentation
- Example correct link: `<b><a href="gmp_define_experiment.html">Define Another Experiment</a></b>`
- Incorrect link: `<a href="gmp/gmp_define_experiment.html">Define Another Experiment</a>`
- For images, continue to include the full path with subdirectories: `<img src="images/gmp/data_acquisition/gmp_data_acquisition_01.png">`

This flattened structure simplifies the build process using a template engine while preserving logical organization in the source files.

## Documentation Style Guidelines

When creating or editing documentation files:

1. Image Formatting
   - Wrap all images in a centered paragraph to prevent large spaces:
     ```html
     <p class="center"><img src="images/example.png" alt="Example"/></p>
     ```
   - Ensure all images have descriptive alt text

2. body Structure
   - Follow the existing template structure with proper inclusion of header and footer elements
   - Use semantic HTML elements appropriately
   - Maintain consistent heading hierarchy (h1, h2, h3, etc.)

3. CSS Usage
   - Use the provided CSS classes rather than inline styles
   - The `center` class is particularly important for properly aligning images
   - Refer to `mainstyle.css` for available styling options

4. Content Guidelines
   - Use consistent formatting for navigation links, examples, and code snippets
   - Keep explanations clear and concise
   - Organize content with appropriate headings and lists
   - Include cross-references to related documentation pages where helpful

## Example Document Structure

```html
[% INCLUDE header.us3/
title = 'UltraScan III Component Name'
%]

<h2>Component Name</h2>

<p>Description of the component and its purpose.</p>

<p class="center"><img src="images/component.png" alt="Component Description"/></p>

<h3>Functions:</h3>

<ul>
   <li>
      <b>Function Name:</b> Description of what this function does.
   </li>
   <!-- Additional functions -->
</ul>

[% INCLUDE footer.us3 %]
```

## Adding New Pages to the Documentation

To include a new help page in the UltraScan III documentation and ensure it's properly indexed and accessible in Qt Assistant, follow these steps:

### 1. Create the Content File

Create a new `.body` file in the appropriate directory (e.g., `gmp/`, `us3/`, etc.) under `ultrascan3/doc/manual`.

Use the existing template structure:

```html
[% INCLUDE header.us3
   title = 'Your Page Title'
%]

<h2>Your Page Title</h2>

<p>Description of the topic.</p>

[% INCLUDE footer.us3 %]
```

Name it logically (e.g., `gmp_new_feature.body`).

### 2. Add the File to `index.body`

To include the new page in the Table of Contents (TOC):

1. Open `index.body`
2. Find the appropriate section or create a new one using `<ul>` and `<li>` elements.
3. Add an entry linking to your new HTML file:

```html
<li><a href="gmp_new_feature.html">New Feature Title</a></li>
```

Use `.html` as the extension because the build process will convert `.body` files to `.html`.

### 3. Understand `manual.qhp` and `manual.qhcp`

#### `manual.qhp` – Qt Help Project File

This file defines:
- The help project’s name, virtual folder, namespace
- All HTML files to be included
- The TOC (Table of Contents) structure
- Index keywords and search keywords

When you add a new page:
- Ensure its `.html` file is listed in the `<files>` section.
- Add it to the `<toc>` and optionally `<keywords>` sections for visibility and searchability.

Example TOC snippet:
```xml
<section title="GMP" ref="index.html">
  <section title="New Feature Title" ref="gmp_new_feature.html" />
</section>
```

#### `manual.qhcp` – Qt Help Collection Project File

This file wraps the `.qhp` into a `.qhc` collection file. It defines:
- The help collection
- Which `.qhp` projects to include
- The output collection file name

You don’t typically need to change this unless you're renaming `manual.qhp` or including multiple help projects.

### 4. Rebuild the Documentation

Run the following from `ultrascan3/doc/manual`:

```bash
make
```

This will:
- Convert `.body` files to `.html`
- Generate the `.qch` and `.qhc` files needed by Qt Assistant

### 5. View Changes

You can use Qt Assistant to review updates locally.

## Testing Documentation with Qt Assistant

After building your documentation, it's important to test it with Qt Assistant to ensure proper formatting and functionality:

1. Launch Qt Assistant:
   ```bash
   assistant -collectionFile bin/manual.qhc
   ```

2. Navigation Testing:
   - Verify all links work correctly
   - Check that images display properly
   - Ensure table of contents is complete and accurate
   - Test search functionality with common terms

3. Display Testing:
   - Test documentation at different window sizes
   - Verify that centered images remain properly aligned
   - Check that font sizes and styles are consistent
   - Ensure code examples are properly formatted

4. Content Validation:
   - Verify technical accuracy of all procedures
   - Ensure screenshots match the current software version
   - Check for any broken references or outdated information

For additional help, refer to the UltraScan support resources.

