# serialtest
serial port test server / client

Tested platform:  Linux.

# Build

$ cd source

$ make remake

# Testing

## Local

[Term 1.]

$ socat pty,raw,link=/tmp/ttyS0 pty,raw,link=/tmp/ttyS1


[Term 2.]

$ ./server /tmp/ttyS0

```
> Opening server on /tmp/ttyS1 0010002,8N2
>       
>
...
> on /tmp/ttyS1 read and echoed 18 bytes: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CC 00.
...
```

[Term 3.]

$ ./client /tmp/ttyS1



```
> Start client on /tmp/ttyS0 0010002,8N2
> 00.00:0 >/tmp/ttyS0 writes 18   bytes:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CC 00.
>	read answer 18 bytes  pack:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CC 00 same, ok.
> 00.01:0 >/tmp/ttyS0 writes 18   bytes:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CC 01.
>	read answer 18 bytes  pack:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CC 01 same, ok.
```

    Ntry.Nmsg:Errors  >PORT 'writes' Nbytes  'bytes:'  hex sec .

      read answer' Nbytes 'pack:' hex seq [ 'same, ok' | 'not equal. err' ].
