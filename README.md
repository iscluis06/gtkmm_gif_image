#  GTKMM GIF Enable Picture Widget

Project that allows an ~~Image~~ Picture Widget to show an animated gif and not only images.
This is a static library, so you only need to make sure to have gtkmm 4  
to use this widget.

It previously used Gtk::Image, but due to the limitations of Image, it was decided to move 
to Picture widget, which allows resizing and filling the GIF into all available space.

### Requirements

* gtkmm 4 (GTK binding for C++)
* pkgconfig (development only)
* libgif (development only)

```shell
apt install pkgconf-bin pkgconf libgif7 libgif-dev libgtkmm-4.0-0 libgtkmm-4.0-dev
```

### Limitations

To set all frame delays, it uses only the first delay from frame 0.
<br><br><br>
If you find this useful, please consider making a donation. Thanks!<br><br>
[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png"  height="50">](https://www.buymeacoffee.com/iscluis06)
