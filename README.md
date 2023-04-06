# 5GReplay


**5Greplay** is a 5G network traffic fuzzer that enables the evaluation of 5G components by replaying and modifying 5G network traffic, by creating and injecting network scenarios into a target that can be a 5G core service (e.g., AMF, SMF) or a RAN network (e.g., gNodeB). The tool provides the ability to alter network packets online or offline in both control and data planes in a very flexible manner.

This repository contains the following folders:

- `src`: C code of mmt-5greplay
- `rules`: set of example XML rules
- `docs`: [documentation](docs/)
- `test`: diversity of testing code

# Documentation

For more details, please refer to https://5greplay.org.


In order to run the DOS attacks related to HTTP2 Post, it is necessary to follow these steps:

1) Compile the plugin in plugin generator folder.You need also to install nghttp2, a library useful for http2 compression and decompression.
The link to nghttp2 is https://github.com/nghttp2/nghttp2.
In my case the commands for this purpose is :
```

gcc -Wall -fPIC -shared  -o lmmt_http2.so -I /opt/mmt/dpi/include/ -I/usr/include/nghttp2 http2_mmt_plugin.c
```

If you need the debug it is necessary to add flags -g -O0.

2) Copy the plugin in the /opt/mmt/plugins and /opt/mmt/dpi/lib/ folder. The command is:
```

cp lmmt_http2.so /opt/mmt/plugins
cp lmmt_http2.so /opt/mmt/dpi/lib/
```

3) Create a simbolic link between lmmt_http2.so and libmmt_http2.so .
```

sudo ln -s  /opt/mmt/plugins/lmmt_http2.so /opt/mmt/dpi/lib/
ln -s /opt/mmt/dpi/library /opt/mmt/plugins/lmmt_http2.so
```

4) Make 5greplay. If debug is needed it is possible to set VERBOSE=1
```

make
make VERBOSE=1
```


5) In order to create the necessary links and cache to the most recent shared libraries found in the directories specified on the command line, in this case libmmt_http2.so, you need to call the following command:
```

sudo ldconfig
```

6) To check that libraries were correctly configured, it is possible to launch this command:
```

ldd 5greplay 
```

7) Compile the rule in 5greplay in the following way:
```

./5greplay compile rules/14.forward-window-update.so rules/14.forward-window-update.xml "-L/opt/mmt/plugins -l:lmmt_http2.so -I/home/frank/mmt-plugin-generator/http2_src"
#./5greplay compile rules/15.fuzzing_path.so rules/15.fuzzing_path.xml "-L/opt/mmt/plugins -l:lmmt_http2.so -I/home/frank/mmt-plugin-generator/http2_src"

```

8) To launch 5greplay it is needed to call the shared library path. Here there is an example of how to do that:
```

sudo LD_LIBRARY_PATH=/opt/mmt/plugins  ./5greplay replay -t ~/Documenti/Nokiahttp2.pcap  -Xforward.nb-copies=1 -Xforward.default=DROP
```


# 
![](https://komarev.com/ghpvc/?username=montimage-5greplay&style=flat-square&label=Page+Views)
