This will display a html page that autorefresh at a givin time

Copy the file capture.html in /usr/bin

from the VxWorks shell run:

```
-> cmd
[vxWorks *]# cd /usr/bin
[vxWorks *]# python3 -m http.server 80
```

In the capture.html there are two variables:
**imageName** - points to the generated image name
**refreshTime** - is the refresh time in miliseconds

Open the your browser at http://rasp_ip/capture.html


