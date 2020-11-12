# menu-admin

## explain

This is a menu for managing Linux applications,Windows like start menu,It supports ```MATE``` and ```GNOME``` desktop environments.

## Function introduction

* The interface is beautiful and easy to use
* Provides search application functionality
* Provide the function of viewing recent reading
* Provide computer management functions, such as shutdown, reboot, log off, lock screen, switch users
* Provides the function of viewing current user information

## Build description
The application currently supports ```Mate``` and ```Gnome``` desktops, with ```Mate desktop``` supported by default.

Open ```meson_options.txt``` file
Modify the value of vlaue from ```true``` to ```false```

```
option('mate',
type: 'boolean',
value: false
description: 'Enable Mate Menus'
)
```
In this way, it can run perfectly under ```GNOME```
## Compile and install

```
meson build -Dprefix=/usr

ninja -C build

sudo ninja -C build install

menu-admin
```
