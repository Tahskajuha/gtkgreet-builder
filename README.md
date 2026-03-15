# gtkgreet-builder

A GTK-based greeter for [greetd](https://sr.ht/~kennylevinsen/greetd/) that aims for highly customizable layouts and styling using [GtkBuilder](https://docs.gtk.org/gtk4/class.Builder.html), allowing users to design their own layouts and styles.

![Minimal Example Screenshot](assets/screenshot_minimal.png)
![Full Feature Showcase Example Screenshot](assets/screenshot_showcase.png)

## Installation

```
meson build
ninja -C build
sudo ninja -C build install
```
Layer-shell support will be enabled automatically if [gtk4-layer-shell](https://github.com/wmww/gtk4-layer-shell) development files are installed.

### Build Without gtk-layer-shell

```
meson build -Dlayershell=disabled
ninja -C build
```

### Launch and Command Line Options

Launch using `gtkgreet-builder`. The following command-line options are available:

- `--help`: Display command-line options.
- `--config, -s`: Path to configuration file. This is currently mandatory as a default fallback is not implemented yet.
- `--layer-shell, -l`: Uses layer-shell if this option is added to the launch command

## Features

### Easy(-ish) Layout+Style Customization

The UI layout is defined entirely through a GtkBuilder `.ui` file while the styling is handled via GTK CSS through a `.css` file.

This allows full control over:

- widget placement
- widget types
- margins and spacing
- fonts and colors
- background images

However, this also means that the user is expected to be somewhat comfortable with GtkBuilder's XML syntax. These are some recommended resources to help as reference:

- [GTK4 Widget Gallery](https://docs.gtk.org/gtk4/visual_index.html)
- [GTK CSS Overview](https://docs.gtk.org/gtk4/css-overview.html)
- [GTK Inspector](https://developer.gnome.org/documentation/tools/inspector.html)
- [gtk4-builder-tool](https://docs.gtk.org/gtk4/gtk4-builder-tool.html)

### Keyboard-First

Enter and Escape buttons are always mapped to `submit` and `cancel` roles respectively.

This ensures the greeter remains fully usable from the keyboard even when buttons are not present.

### Minimal Compositors Supported

The greeter works in minimal environments such as **cage** without requiring a full desktop environment.

### Role-based Behavior-to-Widget Mapping

The greeter defines several **roles**, each representing a specific atomic behavior.

Users can map widgets to these roles by referencing their widget IDs in the configuration file. This allows a high degree of flexibility: multiple roles can be assigned to the same widget, roles can be omitted entirely, or each role can be mapped to a dedicated widget depending on the desired layout.

As of now, 13 roles are provided. Refer to the usage section for more details on each role.

## Usage

The screenshots shown above correspond to full example configurations provided for reference. These examples can be found inside `/usr/local/share/gtkgreet-builder/examples/` after installation (or `/usr/share/gtkgreet-builder/examples/` if installed via a package manager; I mean, I can hope that it reaches that point :P). It is recommended to take a look at these examples to better understand the configuration syntax and customization options.

The examples are designed to work out of the box most of the time. Just modify the command inside `/etc/greetd/config.toml` to run `gtkgreet-builder -s /usr/local/share/gtkgreet-builder/examples/<minimal|showcase>/gtkgreet-builder.conf` with your compositor of choice. For example:

```
[terminal]
vt = 1

[default_session]
command = "cage -s -d -- gtkgreet-builder -s /usr/local/share/gtkgreet-builder/examples/showcase/gtkgreet-builder.conf"
user = "greetd"
```

However, several paths in the layouts and configuration file will break and will have to be modified if the examples are installed to `/usr/share/gtkgreet-builder/examples` instead.

Alongside the examples, empty templates will be found in `/usr/local/share/gtkgreet-builder/templates/` (or `/usr/share/gtkgreet-builder/templates`; fingers crossed, btw). You can start creating your own layouts by copying the contents of the templates folder to `/etc/greetd/` and launching the application using `gtkgreet-builder -s /etc/greetd/gtkgreet-builder.conf`.

### Configuration File

The configuration file acts as the command center of the greeter. It defines the paths to the layout and style files, as well as the mappings between roles and widget IDs. The file is divided into several sections:

#### ui

Defines the UI layout and styling files.

| Key | Description |
| --- | --- |
| layout | For specifying the path to the GtkBuilder `.ui` file |
| style | For specifying the path to the GTK CSS `.css` file |

> Currently only absolute file paths are supported for the above 2 entries.

#### core

Defines the minimum required roles needed for authentication.

| Role | Behavior | Supported Widgets |
| --- | --- | --- |
| initial_answer | Reads the value from the assigned widget and sends it to greetd as the username | Any `GtkEditable`, `GtkDropDown`, or `GtkListView` (single selection only) |
| pam_prompt_answer | Reads the value from the assigned widget and sends it to greetd as the response to PAM prompts | Any `GtkEditable`, `GtkDropDown`, or `GtkListView` (single selection only) |
| read_command | Reads the value from the assigned widget and uses it as a session launch command to be run after auth success | Any `GtkEditable`, `GtkDropDown`, or `GtkListView` (single selection only) |

> The greeter is designed to ensure that the 3 roles above are properly mapped and that the widget IDs correspond to one of the widget types mentioned in the supported column. This is to ensure that an invalid layout does not lock the user out of their system.

#### optional

Defines additional roles that can be assigned to a widget.

| Role | Behavior | Supported Widgets |
| --- | --- | --- |
| question_prompt | Clears the assigned widget and writes PAM prompts into it. If the assigned widget is a `GtkTextView`, the text is appended to the buffer instead of replacing the existing content | Any `GtkEditable`, `GtkLabel`, or `GtkTextView` |
| error_prompt | Clears the assigned widget and writes errors returned by PAM into it. If the assigned widget is a `GtkTextView`, the text is appended to the buffer instead of replacing the existing content | Any `GtkEditable`, `GtkLabel`, or `GtkTextView` |
| info_prompt | Clears the assigned widget and writes info messages returned by PAM into it. If the assigned widget is a `GtkTextView`, the text is appended to the buffer instead of replacing the existing content | Any `GtkEditable`, `GtkLabel`, or `GtkTextView` |
| command_list | Attaches a GtkStringList data model populated with entries from `/etc/greetd/environments` and the configuration's `[session].environments` array to the assigned widget | `GtkDropDown`, or `GtkListView` |
| submit | Attaches the **submit response** action to the assigned widget's clicked signal | Any widget that can emit a `clicked` signal |
| cancel | Attaches the **cancel request** action to the assigned widget's clicked signal | Any widget that can emit a `clicked` signal |
| poweroff | Attaches the **poweroff** action to the assigned widget's clicked signal. This action uses DBus to request the operation from the system | Any widget that can emit a `clicked` signal |
| reboot | Attaches the **reboot** action to the assigned widget's clicked signal. This action uses DBus to request the operation from the system | Any widget that can emit a `clicked` signal |
| suspend | Attaches the **suspend** action to the assigned widget's clicked signal. This action uses DBus to request the operation from the system | Any widget that can emit a `clicked` signal |
| hibernate | Attaches the **hibernate** action to the assigned widget's clicked signal. This action uses DBus to request the operation from the system | Any widget that can emit a `clicked` signal |

> Unlike core roles, these roles are not heavily moderated. They can be omitted, mapped to an unsupported widget or assigned invalid widget ID as the user pleases.

#### state.*

Controls which widgets are visible during different stages of the authentication process. Each state defines a semicolon-separated list of widget IDs that should be visible when the greeter enters that state.

For example:

```
[state.initial]
visible = widget1;widget2

[state.pam]
visible = widget3;widget4;widget5
```

Here, widgets with the ID "widget1" or "widget2" will be shown in the initial (aka the username prompt) state and hidden during the second state when PAM sends authentication prompts. Whereas, widgets with the ID "widget3", "widget4" or "widget5" will be hidden in the initial state and visible in the PAM state.

> Widgets mapped to core roles will be automatically ignored if inserted into any of these lists as the only the greeter can control their visibility. Again, this is necessary to ensure that the user does not get locked out.

#### session

Defines additional session launch commands that can be presented to the user.

| Key | Description |
| --- | --- |
| environments | A semicolon-separated list of session launch commands |

Entries defined here are combined with entries from `/etc/greetd/environments` to populate widgets mapped to the `command_list` role.

## Attributions

The amazing background images used in the example configurations:

- Paper texture by Kiwihug  
  https://unsplash.com/@kiwihug

- Pixel art mountain background by FabinhoSC  
  https://opengameart.org/users/fabinhosc

Power icons used in the full showcase example:

- Bootstrap Icons
  https://icons.getbootstrap.com

gtkgreet and greetd by Kenny Levinsen

- https://github.com/kennylevinsen/
