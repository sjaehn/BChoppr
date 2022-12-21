# B.Choppr

An audio stream chopping LV2 plugin.

Description: B.Choppr cuts the audio input stream into a repeated sequence of up to 16 chops.
Each chop can be leveled up or down (gating) or panned to the left or right. B.Choppr is the 
successor of B.Slizr.

![screenshot](https://raw.githubusercontent.com/sjaehn/BChoppr/master/doc/screenshot.png "Screenshot from B.Choppr")


## Installation

a) Install the bchoppr package for your system
* [Arch](https://archlinux.org/packages/community/x86_64/bchoppr/) by dvzrv
* [FreeBSD](https://www.freshports.org/audio/bchoppr-lv2) by Yuri
* [Guix](https://guix.gnu.org/packages/bchoppr-1.8.0/)
* [NixOS](https://github.com/NixOS/nixpkgs/blob/release-20.09/pkgs/applications/audio/bchoppr/default.nix#L23) by magnetophon
* [Parabola](https://www.parabola.nu/packages/?q=bchoppr) by Erich Eckner
* [openSUSE](https://software.opensuse.org/package/BChoppr)
* [Ubuntu](https://launchpad.net/ubuntu/+source/bchoppr) by Erich Eickmeyer
* Check https://repology.org/project/bchoppr/versions for other systems

Note: This will NOT necessarily install the latest version of B.Choppr. The version provided depends on the packagers.

b) Use the latest provided binaries

Unpack the provided bchoppr-\*.zip or bchoppr-\*.tar.xz from the latest release and 
copy the BChoppr.lv2 folder to your lv2 directory (depending on your system settings,
~/.lv2/, /usr/lib/lv2/, /usr/local/lib/lv2/, or ...).

c) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BChoppr/releases) of B.Choppr. Or clone it 
including its submodules from this repository:
```
git clone --recurse-submodules https://github.com/sjaehn/BChoppr
```

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. If you
don't have already got the build tools (compilers, make, libraries) then install them too.

On Debian-based systems you may run:
```
sudo apt-get install build-essential
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

On Arch-based systems you may run:
```
sudo pacman -S base-devel
sudo pacman -S pkg-config libx11 cairo lv2
```

Step 3: Building and installing into the default lv2 directory (/usr/local/lib/lv2/) is easy using `make` and
`make install`. Simply call:
```
make
sudo make install
```

**Optional:** Standard `make` and `make install` parameters are supported. Alternatively, you may build a debugging version using
`make CXXFLAGS+=-g`. For installation into an alternative directory (e.g., /usr/lib/lv2/), change the
variable `PREFIX` while installing: `sudo make install PREFIX=/usr`. If you want to freely choose the
install target directory, change the variable `LV2DIR` (e.g., `make install LV2DIR=~/.lv2`) or even define
`DESTDIR`.


## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Choppr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BChoppr
```

to run it (pseudo) stand-alone and connect it to the JACK system.

Note: **Jack transport is required to get information about beat / position**


## Usage

The plugin slices a stereo input stream, amplifies or silences the individual slices and send the whole
sequence to the output. Although this affects only the audio signal, it needs a playback running
(Jack transport).

In addition to the global controllers, the interface is divided into three parts: step controls,
monitor and step shape.


### Global

* **Bypass** : Bypass B.Choppr
* **Dry/wet** : Dry / wet mixing


### Step controls

* **Step markers** : Defines the size of each step. Drag markers to relocate. Right click to switch between automatic and manual placement
* **Step level sliders** : Sound level for each individual step
* **Step panning dials**: Sound l/r panning for each individual step
* **Sequences per bar** : Number of sequences in one bar (1..8)
* **Amp swing** : Sets all level sliders to a swing pattern (0.001 .. 1000.0). Values lower than 1 mean reduction of odd step level. Values higher than 1 mean reduction of even step level.
* **Steps swing** : Sets all automatic markers to a swing pattern (1:3..3:1)
* **Auto markers** : Sets all markers to automatic placement
* **Number of steps** : Number of steps in one sequence (1..16)

### Monitor
* **On/Off switch** : Switches monitor and monitor <-> plugin communication on/off. Reduces CPU load.
* **Monitor** : Visualization (l + r signal) the input / output signal for a whole sequence. Use mouse wheel or drag to zoom in or out.

### Step shape
* **Blend** : Select between linear (for trapezoid shapes) and sinusoidal blend between the steps
* **Attack** : Time (fraction of the respective step length) to increase the level at the begin of each step
* **Decay** : Time (fraction of the respective step length) to decrease the level at the end of each step
* **Monitor** : Visualization of a single step


### 🔗 Shared data

If you use multiple instances of B.Choppr you may be interested in sharing the plugin settings between
the instances. Click on one of the four shared data fields and the respective plugin instance is linked
to the shared data. The plugin instance data are copied to the shared data field if no other plugin
instance is linked to this shared data field before (otherwise *vise versa*).

If you now click on the same shared data box in another plugin instance, both
plugin instances get the same data.

Click again on the selected box to unlink the plugin instance from
shared data. The plugin now shows the host-provided data.

Note: Shared data are unlinked from host automation.


## Internationalization
B.Choppr now uses the dictionaries of the new B.Widgets toolkit and all labels
are now automatically shown in your system language (if translation is 
provided). The dictionary for this plugin is stored in 
src/BChoppr_Dictionary.data. If you want to add a translation to your language, 
simply edit this file in your text editor und use the (POSIX) language code 
(format: language_TERRITORY) of your language. 

E. g., if you want to add a french translation of "Help", simply change
```
    {
        "Help",           
        {
            {"de_DE", "Hilfe"},
            {"it_IT", "Aiuto"}
        }
    },
```
to
```
    {
        "Help",           
        {
            {"de_DE", "Hilfe"},
            {"fr_FR", "Aide"},
            {"it_IT", "Aiuto"}
        }
    },
```

Once you changed the dictionary, you have to re-build the plugin. And please
share your translations with other users by either submitting a git pull 
request or notifying me (issue report, e-mail, ...).


## What's new

* Migration to the new B.Widgets TK
* New internationalization


## TODO

* Moooaaaaaarrrrrrr presets


## Acknowledgments
* Thanks to LAM and sahaathyva for translation


## Links

* Tutorial video: https://youtu.be/PuzoxiAs-h8
