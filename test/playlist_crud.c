/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>
#include <ulfius.h>
#include <orcania.h>
#include <yder.h>

#include "unit-tests.h"

#define IMAGE_BASE64 "/9j/4AAQSkZJRgABAQEAYABgAAD/4QBORXhpZgAATU0AKgAAAAgABAMBAAUAAAABAAAAPlEQAAEAAAABAQAAAFERAAQAAAABAAAAAFESAAQAAAABAAAAAAAAAAAAAYagAACxj//bAEMAAgEBAgEBAgICAgICAgIDBQMDAwMDBgQEAwUHBgcHBwYHBwgJCwkICAoIBwcKDQoKCwwMDAwHCQ4PDQwOCwwMDP/bAEMBAgICAwMDBgMDBgwIBwgMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDP/AABEIAJYAlgMBIgACEQEDEQH/xAAfAAABBQEBAQEBAQAAAAAAAAAAAQIDBAUGBwgJCgv/xAC1EAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8/T19vf4+fr/xAAfAQADAQEBAQEBAQEBAAAAAAAAAQIDBAUGBwgJCgv/xAC1EQACAQIEBAMEBwUEBAABAncAAQIDEQQFITEGEkFRB2FxEyIygQgUQpGhscEJIzNS8BVictEKFiQ04SXxFxgZGiYnKCkqNTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqCg4SFhoeIiYqSk5SVlpeYmZqio6Slpqeoqaqys7S1tre4ubrCw8TFxsfIycrS09TV1tfY2dri4+Tl5ufo6ery8/T19vf4+fr/2gAMAwEAAhEDEQA/AP38ooooAKK4fTP2lvh/rPx+1P4V2vjHw7cfEfRtLi1q98OJeodRtrORtqzNFndtztz3USREgCRC3cUAFFFFABRXzX/wUN/4KzfBL/gmN4Whu/id4mYa9qEPn6X4Y0iIXmuaqmWG+ODcoSPKOPOmeOLcpXfuwD+Mf7UP/B4R8aviFqN1a/CT4f8Ag34d6M25IrzWfM1zVmAb5ZBgxW8RK9YzHMATgOcZOc6sY7mkacpbH9GFFfyZ3v8Awcj/ALcEl0Zh8dLiLk7Y4/B+geWoPbBsSTjtkk+9T+Kv+Dkv9trxNp0EH/C5P7LWOIJI1h4V0eGS4P8AfZmtWIb/AHCg9qz+tQL+ryP6xaK/k38Af8HLf7anw81S3mk+L0PiS1hfc1jrXhjS5YZ/Znit4psf7sq19+fsS/8AB5Tput6rZ6P+0N8Nk8PrPIEk8T+DGkuLODcwAaXT5maZI1GSzRTTuegiqo1oMmVGSP3Morj/AIEfH/wT+0/8LNK8bfD3xRo3jDwprUfmWepaZcrPDJ/eQ45SRDlXjcB0YFWVWBA7CtjIKKK8P/4KFf8ABQf4d/8ABMv9m+8+J3xKuNSGiw3kOm2llpkCT6hq13LuKW9vG7ojSbElkO51ASJyTxQB7hRXNfBv4v8Ah39oD4TeG/HHhHUotZ8L+LtNg1bSr2NGQXNtNGJI22sAykqwyrAMpyCAQRXS0AFFFFABRRRQB/H/AP8ABRv9uLx18Jv+C9nxd+M3hPUn03xp4C+JF9Z6ZO7MUePTXOmCCUKwLQS29v5UiZG6ORl71/VX+w/+1x4d/bv/AGTPAnxc8K7o9H8b6Yl6LZ23vYXCs0Vzau2BueC4SWFmAwTGSOCDX8in/BaH4Y6h8Jf+Csf7Rej6pE0dzcePtU1lFZduYNQnbUID9DDdRkHuOa+/P+DST/grRpP7OXxO1n9nP4ha1a6V4X+IF8NT8I317MsUFlrLKsctmXYcC6RIvLBYKJodqgvcVlGWrRpKOl0f0iV8H/8ABdD/AILRaH/wSo+CUOm6GdP1z4zeMoH/AOEb0aU+ZHp8IJV9TvFBBFujZVEyGnkBVflSZ4vdP+Cjn/BRb4f/APBM39nDU/iB46vI5J1R4dD0OKdUvvEd7tyltADk9cF5MFY0y7cDB/kI/a7/AGsvGn7bv7RXin4peP76O+8TeKrozSpCCtrYQr8sNpbqxJWCGMKiAksQNzFnZmZVqnKtNyqNPmd3sUvH3xU8RftC/FjWvGnjvxDqHibxV4kuTd6nquoS+ZcXcmAoyeAqqoCqigIiKqqqqoAmuZ9H0y2AWRXfHNeZ3Wp+SOGx3zXQeAPhD4y+LLL/AGDoWoXtu5x9qKeVbD/tq+EOPQEn2rzqjjFc9RpLu3Y7qfNJ8kE2+yLGsa5ZyhvLVRW9c+JNGuU2ooXjHSvTPAH/AATU1LU0WTxN4ohsf71tp0PnMP8Ato+0Z+iH61reMP8Agl5d+Q0nhnxaryY+W31W32hj/wBdoun/AH7NeVLPMvU+R1Pwdvvt/wAA9KOT45x51T/FX+658867Pa8+WV/CufuZxmuw+J37MnxC+EaySa14a1D7HH/y+2a/a7Yj1LR52j/fCmvPFvPPXKsGHsc161GdOrHnpSUl5O55laNSnLkqRcX5qx9Sf8Euf+Cr/wATP+CUnx6h8VeC7ptW8L6lKieJ/CN1clNP8RW44PY+TdIMmK4VSyHhhJG0kT/1v/sW/tm+Af2+/wBnTw/8UPhvqran4c16LmOZRHeaZcLgS2lzGCfLniY7WXJB4ZWdGV2/h9Dbh619tf8ABEP/AILHeJP+CSn7RTXlxDeeIPhL4ukjg8XaBE2ZVUcJqFmCQouoQT8pIWaPdGxU+XLF3U5W0ZyVIX1R/X5X813/AAeUftaXHxJ/bc8E/CGyvRJovwy8Prqd9bxuRjVNQbcRIM4JS0htWQkZUXMmMBjn9+9F/bk+EfiP9lKb44af8QPDt58KLfTH1ebxHDcb7WGBBlw6gb1mU/IYColEn7soH+Wv40/24/2pNV/bW/a3+InxW1gXMd3481y41KK3ncSSWNqTstbUkdfJtkhhB9IhWlSVlYzpx1uf0P8A/Bnp+0vcfFv/AIJoa14Bv7hZrr4T+Kbmxs4wuDFp14q3sWT3P2iW9A9FVR0GB+sdfhD/AMGQ9jdR+Ev2krlkYWct94diibHymRYtRLge4V4/zFfu9VR2JluFFFFUSFFFFAH883/B4z/wTxuvCnxd8K/tKeH9P3aL4pgh8L+LnhQf6PqEKt9iupDkkia3Bg3bQqmzhBO6VQfxa0bwxp2qwbprpYZe6OmVI+tf3EftH/s7+Ef2s/gX4o+G/jzR7fXPCXi6xaw1G0lHVThlkRuqSxuqSRyD5kkRGUhlBr+Sv/grp/wRa+KX/BJz4l3DapZ33ib4V6jcsmheM7W3JtZFJ+S3vNoxbXWP4Gwsm1jGWCsE56tN7xN6clsz59+KHxP1/wCJc+n3Pirxh4l8ZXuk2MemWM+tapPqEljaR/6u3iaZ2McK9o1wo7CpP2c/2evEH7VHxQh8MeHVjjZYjdX19MCYNNtlIDSvjk8sqqo5ZmAyBlh5hJfuR941+l//AATI8Br8Bf2ObrxndQsupeNrhr92I+b7JBujt4/oSZZB7Te1fN8QZjPAYR1Yazk1GN+76/JXZ9BkeAjjcUqc9IJXl6L/ADdkXfhz/wAE+fAfwhNvHaaCfFWsblQ6jq0YuXkkPAEcP+rj5+6FUvyBuY817l4w/Y0+I/hTwqdY1nwX4w0PSYkDNeXuh3Vtawr2LSNGFUdMEkZ7V+pv/BGj9jLTPA3wB8OfF3xHbQ6l48+IFgmsWc0nzpoenXCbreG3HRHeBleRwA5MhQkqoFfTHxi/ay+FHwH8U6L4d8ffEjwD4P1rxQfL0vTde161sLnVMnZ+6imdWkBb5OAQWIXqQD5GF4VxeKpKvja752rrS9vW7/BWseniOJMNhqjo4SiuRab2v9y/F3ufznyeGL7RtQWGRWIbnOOtdZa6fYeH9JF9q15FZxKON7Y3nGelfaH/AAVq/Y50j9mn4w6Xrnh+zt7Hwn44aV7eziTbHpd9GA0sCDosUit5kaDhSkygKioo4/8A4Ix/sq+Hf2sfjR4w+MnxAhsJPh/8L5Fg0621EqLGa9WPzXnn3/J5VvFtkIb5S8qsSPKIPzdPJcXWxzwEtHHd9Ld16r/Lc+glm2Fo4NY6Oqey637fJ/5nm3ww/Zy8afETwYmtaP8ADf4ialo91GJYb2Hw5dtDcxkZDRER/vVI6FAwPua8B+PP7HPwz+KF3fQaxoWn2WvwO0VwFjNjqFvIOokxtkVhkHa/ryK/or/Z8/a7+Ff7WVnq1x8MfiJ4L+IEOgzLb6i2gaxBqH2F2LBBJ5TNtD7HKk8OFJUkDNeB/wDBWj9inwL+1p8Lf9MtLfTfiDp8WNG8Q28eLm0AyRFMRzNbkk5jbpuLJtbDV9LU4JnQ/e4KvJT+6/3bfifPw4vhW/dYujFx++337n8qP7Vn7Hl5+zvenUNOvJNY8NySBPNcD7RZMThVl24DKeAHUAZOCAdu7x21kXjNfoR4x8NS3h1zw34is9t1ZzXGk6nau24LIjNFKme+GBww9iK+Cvir8P7z4TeN7rSLos8cbF7Wcj/j5hJ+V/r2I7EHtg17GS5hUrRdDEP34/K//BR5Wb4GFGSrUF7kvw/4DGWt5HbQtGWb7PI6yyRBsJI652sw6EjJwTyMmqPiDUo9QvZJo444Y/7qjCrWe10FG5mx7k1+wP8Awb4/8G6nij9p34k+HfjL8dPC9xoXwj0d4tU0jQdYtTHceNJRh4S9u4yth0dmkGLhdqqrRuzj3oU3c8SVTQ/Vn/g2b/YY1D9iL/glr4ZbxBZvY+LfihdyeNtUt5UAms47mOJLOBuAwK2kUDsjcxyTSrjIOf0Eoors2OUKKKKACiiigAqn4i8O6f4v0G90rVrGz1TS9Sge2u7O7hWa3uonBV45EYFWVlJBUggg4NXK81/bD/aS039kH9mLxt8StWj+0WvhPTHu47fdt+13BwkEGe3mTPGme2/NRUqRpwc5bJXfyLp05TkoR3bsj8M/+DkP9jj9jz9lvXNJ0X4bfC/R9F+M2v3I1XUIdFv7i30rSbP+HzbBX+zIZSDsijjT5FckYKZ4/wCCps9Z/ZJ0OObatnHpcccuAAF+QdB/SvkP40/ErXvj98Vde8deMNQfVPEfiS8l1HUbqTgNI5ztUdFjRQFVRwqKoHAr7M+G3wP8Ufs1+AvAvhn4hae2mnx54Zi8SafE2dy2dxLIIkkBA2zom0vH95PMQNgmvyriqtVxdFV7O0JJ+i/r735H6Xw3Rp4Sr7G6vJW9X/X4H7Zf8E0/jn4Z+JX7Avwp1DTtWsXh03w5aaNcgyqpiuLKMWkyleo/eQtjPVSD0Ir8k/8Ag4u/4I7eKv22v21ofit4J+KXhGDw7rOmWWm61Ya/qFxCujSWodFkgCROkkRV2bZkMJZJCMhyRpfByLUP2b/D15pvhvXtS03T7+Y3U9vGUaF5SAN4V1baxUKCRjO0ZzgY4n4xftEpD4htLHUNcibWNQYpZpe3w86duPliVyMnkfKg7jjmuqnx9F0Y08PTcppddFp1vr67HLPgmUarqYiooxb6avXp09Nz079t39oqT4hfBLwL4Hg1q81ZPAGnWWm2WpX4P2nUJYLUWxupFJJ3uNzsWPLOeo5rN/Zz1TRtM/4JpfGD9mrxJ4ivNDs/iXbXT6V4osLditnPOFbZdRqS3lF0VH2khoi4ypxn5l1zxDdatreJiytG3Ibrmu8stdt9E8NfbtQvIbCwtxme4nmWKKIerOxAH4mvE/t7MKOI+tqScpaNW0t0Xf8AHqex/YuBq0PqtmorVO+t+/b8D1L/AIN3f+Cd/iv/AIJvfGfx58TPF3xB8M65d+K9FOgWVpoN9PeWtwjXUVw95M8sceZAYFWMYYhZpSSpOD+lHxN+L1nf6dcTTXkbswJZ2f8AnX5IeGtf03xPoy6z4W12RopshdQ0XUGUSlTg5aM4bBB4bI9q4z45a/8AEvxP4XvNLk+I2rXGiXSGO4t5o4I5JYyOY2kjRW2kcEDG4ZByCRX1uD42oVPdxEXGXbpf+u6Pl8XwhXp+9QkpR79f6+Z5v8VvGlp8VPjl8QPFGnMsml6xr13c2ki/dmhMrBJB/vqA3/Aq+evj/ZWGsa1aQ6hYQ6lZqrtNE4IbGU5VlIZTx/CQSMjPOR6st1FoGii2hb5FGDjgGsrxp+zR40v/ANniP41PpLP8PbnxJL4QS9Gd0d5HAk+4rj/UvvaNZASPMgkQ4baG4cJKc8RKuvN6HdiYwhQjRfkj91v+CM3/AAT3/Yp8R/Avwt8ZPg78H/DcetXSp9oudcuZvEGpeH9Sh2mSNZLtn8iZGwRJCkRZGRwAHAr9FK/nN/4N7/22bv8AZG/a30vwrqF4y+CPibLFo2oxO37u2vGO20ugOApEjeWzE42SuSCVXH9GVfc5bjViKV3utH/n8/zufGZhg3QqWWz1X+XyCiiivROAKKKKACiiigAr4H/4OOvC/i7xp/wT1tdL8I6JruvNeeLdPGqW2lWUt5KLUR3DKzJGrNt+0LbDOMbmX1r74rh/2kvjbZfs6fA/xF4yvlWWPRbUyRQk4+0TMQkUZPYNIygnsMntXHmMabw0/avlik232S1OzL5TWJh7NXldJLu3ofh/+wV/wS38O/AS+0f4wftaTWvgrwxp8i3ugeA9SiMmseJJkwySXdoAXjtVbbmFwDI3EwjiBSf3b9v/APa28M/8FOrXwmvh/wAL6po9p4V1CeXSdamlQapctKFieFUXdGkLOillLOXaGM5TYQ/xn8e/iNr37UvxK13xZ4u1y4up7mUyPLI+1QP4Qo6KijhVGABgAYr65/ZP+GNroHhhLlYgtvpVstlZgrj5wo8yT68499zd6/FeIeKKlHBujh1yxlvpeT9X92i211e5+u5Hw7CtilWrvmlHbol6L79XvpseB3niufRvE914T1iaO38QaWoAx/q7uMjiaInqCOo5wQR2zXn/AMR/gcPFRaK8s7XWNNuss9tdoJFJPUjPQ+45r2v9sT4EDxxeC+jVrfULEeZbTRnbJHz0B69a8Bsvj34h+Hl1HpviTT5NSjhbCTwkR3BH0OFf6ZDcdG614uWzlUpqthmubqtte6/rQ9XMIxhN0sR8PR/5/wBalxfhPcJDp1vAtxJ9njMUjXM/mS4B+XL9WwuFy3J2jJJ5qh8Uf2edL+IOvR3XibVprjRdNCfZtJGY4A4UbpJDnLsTnHTA49c3b79rDRruOZbYxLcLyqXH7h19yrYYflXmPib4yHxLdu32qTUpM5ENkPMRPq33F/4Ewr1sPTxjnzP3bdeuv5ep5lephFGy978tPz9D1Sw8UaD8MPDV1Z6PHbx7MbY4htUZ4wB7YryT4i/Ei58W35RyGaQ4W3h7n1Pv9elZtl/afiycxwqsMecMUJZV+r8bj7KAB6nrXb+DfhlDpK72UtIR8zkfMf8APpXVGlSw7c5u8jmlUqV1yQ0ieQ+JPB2rLK0M37rcm9dnIYH39q/ST9gL/grp8EZP2INN/Zf+PPwwvNM8HW+mPod3q+kwNfabfo0jTG8nhQ/are6MrCYyW6ykXGZk8n5VT5d8Q+BF1vwtMI1Aurb97CcfeOOV+jdPrj0rxO3u7WHxOysu0zLvVfoeR+or2crzqpDmdNLs1Y8nMsphOyqN90z6C/aY/wCCU3jj9nC8tvFnwwk1H40fCHWmNz4b8YeE7d9TnRdxAhvIrVS0VxGylDIqiJmA5jcmGP8Ao4+HWoalq3w+0K61q3az1i60+3lv4CMGGdo1MiY9mJH4V+Ff/BKn9tO+/Y1+NOkTzag8fgHxTexWPiWzd9ttEJWWJNQA6LJCdjMwBLxK6EE7Cn72V9hw3WoVlUrUrp3Sceit28nc+V4gp1qThSq2a1afe9t/NWCiiivpz50KKKKACiiigAr8uv8Ag4P/AOCgGl+D9Dtfg34fC3/iK12a/wCIJg+YdLtxDJ5UDhTkzSBhLtONqiNjnzFr9Ra/k6/4KhfGv4i6L+398bNP8XabceH9avfFOotJa39ufPksWncWjKzfeha2WERug2tGFwSOa8TPozqYb2EVpLR+n/BPYyWUYYj20nrHVepmfEj4y69oHhd7eSGyZbuCKeSMRkMp4fCkk/rmv1A/ZR+Len+KvgN4S1S3ulkXUrVp4ST8s2X3sP8Ax4/5FfiD4r+NWteIwft1xDONgjA8pV2qBgAbcdAB1r69/wCCWfx31bw98B7iy1KHUf7D0vXrq20nUDCwtpn8uGae1SQja0kXnxsyA5VZYicBxn8s4q4blVwCnFaxd3bs1b87H6Vw3n6p41wk9JKyv3Tv+Vz9PvFy2erWUZkhjijuARGzrxI3f64r52+O37PNnqltM3k+cz5b5l+Xk+v51lzfGG1i1W5mtY4VjuGFzHIrbpA2wZz2XLgkjuD+J6K7+OEetaZDDNN5ytH+8bylTae/Gdzeua/PcLhcRhZKUGfcYjFUMTFxmfLPib4FS2N00e9lVT8qyKJFH03Amqlh8F/MdftUsk0aniPhU/75GB+de4eKtdt9dvpJI41EbEAcYZgO5GeD+dYTrGo/hAr6ynmFZxV9z5mpgaSloc7o/guHTo1WONVVRwAMYrWWxSAcVNNdxxjrj61nX2uxxr96s+ac2VaMUJqlx9mgba2PpXyD8WfFU6fHd9M01Y5JIWeeTcTtQScqOPqeP9mvpTxd40htLOU7x09elfBGofF680T4qeIr9fJa+lv5PmlUtsRfljwMjgoFYHkENnvX1XDuClOU5W2X5nzefYyMFCN+v5H0rZfGzXNG8N6npt1p+lX1nGfsk8bK8bOHTPDbjtPUZwcjtX9Nf/BOj9q/Rf20/wBjPwH4+0jUf7QnvtNitdXVz+/s9ShUR3cMo6hlmViCeHVkdcq6k/yJD4/6rfR3SzNZv9umWeY+RgswXaOh4GPSv2w/4M8Pixq3iXQv2gPDMn2g+H9PvdE1i3UA+RDe3MV3DcHPTe8VpaZHXEYz2r7TI8K8NWkkrc2/y/pnyecYhYiine/Lt8/6R+1VFFFfVHzIUUUUAFFFFABXnf7Qn7JHwv8A2sdCh034mfD/AMI+OrW1DfZhrOlw3clmWxuMMjKXiY4HzRlT716JRQ0noxptao+K7f8A4N2v2MrbXV1D/hR+kyTK+8Rza1qk1tn3ga5MRHsVx7V6l+1F+zJ8AfCn7EWqeDfGOleEfhx8I/DsJuIJLGKDSLfw1MWby7m02qEiuPMlIUKp81pWRlkErI/tHi/x1beC7bzJ7PWrzjITT9MnvG/8ho1fgb/wXU/bX1z9pj9sO78J6gus6B4D+HsiW+jaRqtrJp7XN2YgbjUJYZAC0hZ3hjY5CxJlNvnS7vLzPEUcNh25RTvpbo/XyPRy7D1cRXSjJq2t+q9PM+VPiJ+1V4b+GXxS1zStJ1HWvFXhHT7po9M8THQ5rBNUg4Kym2fM0PUgq6g5GeAcV03gj9qrwv4x2rZ6xYzSNz5ayjePqp5/OvLviXdaDa+CIYhdwSXGoXMcLHzB8iEjPf3rifi58L/C2p6PYSRNZrM0AZ9rAMje1fnUsvwdfVpxv21X46/ifeRx2LpaXUrd9H+Gn4H2Ba/EG0uo9yzKffNVdV+IlraoczLx71+dOteOte+Fv/IP8UzLCvAjmuBKv0AYkj8CKo6f+0J4o8dsYZNdhtB90+UwVmH1Ykj8K0jwk2vaRmuX5kS4nS9yUHzH3X4r+Pmn6PCzzXcMMa/xO4VR+deV+L/20fD9izLHqC3T9hbI036qCB+Jr5wbwgmpf6Veakl5PjO+W48xvzJNV9G0e1u7i4/fQbYz/eFejh8hwsFeTb/D/M4K2dYmbtFJH6If8EePCPwf/wCCh/7Wdj4V+KnxGtPCGnxzRNp/ha4M1le+OJSwxaRXeBDEpJVSiSfaZAWESpjzV/fL9pr/AIJRfs5fth+G9H0v4hfB/wAG6xb+HrGHStKmtbU6Xd6ZZQgiK1gubRopordASFiRwi54UV/H7rGl6ZFAwluLYLjPMgHNftj/AMEnv+C2Xxa+Mv7NVpo/iDxFrXiXXvBtx/Yx1U232ufVbdUVoJZpdhMk4UmN3Yl3MQdyXdifqsrhQpxcKUbfqfM5lKtUlz1JX/Q+19I/4Ngv2KdJ1Y3R+FGpXS7tyW9x4y1t4UOc9PtYLD2YkHvmvs74F/s+eBf2Y/h7beE/h34R8PeCvDdqxkj07RrGO0gMhADSMEA3yNtG52yzEZJJr4n8P/tu/FvWQrR2nihlbof7Ebn/AMhV2ejftOfGC7Cn+zvEhz/e0Jv/AI1Xrq26PM1ejPtWivnn4f8AjD40eMrfzlt4baPv/aFmLY/kUBP4V6Lo1l8Ttim7vfCXuDDKx/8AHSBVXJseg0Vz9hF4pCf6VcaBu/6ZQS/1eimI6CiiigAooooAKjubaO8gaKaOOWOQYZHXcrD3BqSigDmZvgv4OuLr7RJ4T8MyTg5EjaXAWB+u3NUX/Zt+HUlwZm8A+C2lY5LnQ7bcfx2V2lFTyR7Fc8u5g6V8LfDOhD/QfDmg2eOnkafFHj8lFQ+JPg74R8ZR7dY8K+G9VXpi80yGcf8Ajymukop2Qrs8m1T9gn4F65IWvfgv8Jrxj1M/hDT5CfziNQP/AME8/gDIqq3wN+D7LGMKD4M075fp+5r2Cilyx7D5pdzznw5+x58I/B0qvpHws+HOlun3Ws/DVlAV+hWMV6BZWEGnW6Q28MNvDGNqJGgVVHoAOBU1FVZLYm7YUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFABRRRQAUUUUAFFFFAH/2Q=="

#define AUTH_SERVER_URI     "http://localhost:4593/api"
#define USER_LOGIN          "user1"
#define USER_PASSWORD       "MyUser1Password!"
#define USER_SCOPE_LIST     "taliesin"
#define ADMIN_LOGIN         "admin"
#define ADMIN_PASSWORD      "MyAdminPassword2016!"
#define ADMIN_SCOPE_LIST    "taliesin taliesin_admin"
#define TALIESIN_SERVER_URI "http://localhost:8576/api"

#define DATA_SOURCE_VALID       "dataSourceTest"

#define PLAYLIST_USER_VALID    "playlistTest"
#define PLAYLIST_USER_INVALID  "playlistTestInvalid"
#define PLAYLIST_ADMIN_VALID   "playlistAdmTest"

struct _u_request user_req, admin_req;
char * user_login = NULL, * admin_login = NULL;

START_TEST(test_list_playlist_ok)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_ok)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{ss ss ss ss s[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "cover", IMAGE_BASE64,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");

  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_invalid_cover)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "cover", "invalid",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_scope)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", "description for "PLAYLIST_USER_INVALID,
                                  "scope", "all",
                                  "cover", IMAGE_BASE64,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_description_type)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssisssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", 42,
                                  "scope", "all",
                                  "cover", IMAGE_BASE64,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_media_empty)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssssss[]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", "description for "PLAYLIST_USER_INVALID,
                                  "scope", "all",
                                  "cover", IMAGE_BASE64,
                                  "media");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_name_exist)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "all",
                                  "cover", IMAGE_BASE64,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_adm_ok)
{
  char * url = msprintf("%s/playlist?username=%s", TALIESIN_SERVER_URI, user_login);
  json_t * j_playlist = json_pack("{sssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_ADMIN_VALID,
                                  "description", "description for "PLAYLIST_ADMIN_VALID,
                                  "scope", "all",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");

  int res = run_simple_authenticated_test(&admin_req, "POST", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sssssssis[{ssss}{ssss}]s[]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "elements", 2,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/FreeSWSong.ogg",
                                  "stream");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_playlist, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_not_found)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_cover_ok)
{
  char * url = msprintf("%s/playlist/%s?cover", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_adm_cover_not_found)
{
  char * url = msprintf("%s/playlist/%s?cover", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_error_input)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sis[{ssss}]}",
                                  "description", 42,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/eternal-flame.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_error_media)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_updated_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sssssssis[{ssss}]s[]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "elements", 1,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/FreeSWSong.ogg",
                                  "stream");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_playlist, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_adm_no_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_ADMIN_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/eternal-flame.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 403, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_ok)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_input)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{ssss}",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_media)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 403, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_has_media_ok)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                  "data_source", DATA_SOURCE_VALID,
                                  "path", "/fss"), * j_result;
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
  
  url = msprintf("%s/playlist/%s/has_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "/fss/free-software-song.ogg");
  j_result = json_pack("{ss}", "path", "fss/free-software-song.ogg");
  res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 200, j_result, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
}
END_TEST

START_TEST(test_playlist_has_media_not_found)
{
  char * url = msprintf("%s/playlist/%s/has_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "short/short1.mp3");
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 404, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
  
  url = msprintf("%s/playlist/%s/remove_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "/fss");
  res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
}
END_TEST

START_TEST(test_delete_playlist_not_found)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_delete_playlist_error_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 403, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_delete_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_delete_playlist_adm_ok)
{
  char * url = msprintf("%s/playlist/%s?username=%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID, user_login);
  
  int res = run_simple_authenticated_test(&admin_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Taliesin media server test playlist CRUD");
  tc_core = tcase_create("test_playlist_crud");
  tcase_add_test(tc_core, test_list_playlist_ok);
  tcase_add_test(tc_core, test_create_playlist_ok);
  tcase_add_test(tc_core, test_create_playlist_error_invalid_cover);
  tcase_add_test(tc_core, test_create_playlist_error_input_scope);
  tcase_add_test(tc_core, test_create_playlist_error_input_description_type);
  tcase_add_test(tc_core, test_create_playlist_error_input_media_empty);
  tcase_add_test(tc_core, test_create_playlist_error_input_name_exist);
  tcase_add_test(tc_core, test_create_playlist_adm_ok);
  tcase_add_test(tc_core, test_get_playlist_ok);
  tcase_add_test(tc_core, test_get_playlist_not_found);
  tcase_add_test(tc_core, test_get_playlist_cover_ok);
  tcase_add_test(tc_core, test_get_playlist_adm_cover_not_found);
  tcase_add_test(tc_core, test_set_playlist_ok);
  tcase_add_test(tc_core, test_set_playlist_error_input);
  tcase_add_test(tc_core, test_set_playlist_error_media);
  tcase_add_test(tc_core, test_get_updated_playlist_ok);
  tcase_add_test(tc_core, test_set_playlist_adm_no_credentials);
  tcase_add_test(tc_core, test_playlist_add_media_ok);
  tcase_add_test(tc_core, test_playlist_add_media_error_input);
  tcase_add_test(tc_core, test_playlist_add_media_error_media);
  tcase_add_test(tc_core, test_playlist_add_media_error_credentials);
  tcase_add_test(tc_core, test_playlist_has_media_ok);
  tcase_add_test(tc_core, test_playlist_has_media_not_found);
  tcase_add_test(tc_core, test_delete_playlist_not_found);
  tcase_add_test(tc_core, test_delete_playlist_error_credentials);
  tcase_add_test(tc_core, test_delete_playlist_ok);
  tcase_add_test(tc_core, test_delete_playlist_adm_ok);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  struct _u_request auth_req;
  struct _u_response auth_resp;
  int res;
  
  y_init_logs("Taliesin test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Taliesin test");
  
  // Getting a refresh_token
  ulfius_init_request(&auth_req);
  ulfius_init_request(&user_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/token/", argc>7?argv[7]:AUTH_SERVER_URI);
  u_map_put(auth_req.map_post_body, "grant_type", "password");
  user_login = argc>1?argv[1]:USER_LOGIN;
  u_map_put(auth_req.map_post_body, "username", user_login);
  u_map_put(auth_req.map_post_body, "password", argc>2?argv[2]:USER_PASSWORD);
  u_map_put(auth_req.map_post_body, "scope", argc>3?argv[3]:USER_SCOPE_LIST);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    json_t * json_body = ulfius_get_json_body_response(&auth_resp, NULL);
    char * bearer_token = msprintf("Bearer %s", (json_string_value(json_object_get(json_body, "access_token"))));
    y_log_message(Y_LOG_LEVEL_INFO, "User %s authenticated", USER_LOGIN);
    u_map_put(user_req.map_header, "Authorization", bearer_token);
    free(bearer_token);
    json_decref(json_body);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication user %s", argc>1?argv[1]:USER_LOGIN);
  }
  ulfius_clean_request(&auth_req);
  ulfius_clean_response(&auth_resp);
  
  ulfius_init_request(&auth_req);
  ulfius_init_request(&admin_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/token/", argc>7?argv[7]:AUTH_SERVER_URI);
  u_map_put(auth_req.map_post_body, "grant_type", "password");
  admin_login = argc>4?argv[4]:ADMIN_LOGIN;
  u_map_put(auth_req.map_post_body, "username", admin_login);
  u_map_put(auth_req.map_post_body, "password", argc>5?argv[5]:ADMIN_PASSWORD);
  u_map_put(auth_req.map_post_body, "scope", argc>6?argv[6]:ADMIN_SCOPE_LIST);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    json_t * json_body = ulfius_get_json_body_response(&auth_resp, NULL);
    char * bearer_token = msprintf("Bearer %s", (json_string_value(json_object_get(json_body, "access_token"))));
    y_log_message(Y_LOG_LEVEL_INFO, "User %s authenticated", ADMIN_LOGIN);
    u_map_put(admin_req.map_header, "Authorization", bearer_token);
    free(bearer_token);
    json_decref(json_body);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication user %s", argc>4?argv[4]:ADMIN_LOGIN);
  }
  ulfius_clean_request(&auth_req);
  ulfius_clean_response(&auth_resp);

  s = taliesin_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  ulfius_clean_request(&user_req);
  ulfius_clean_request(&admin_req);
  
  y_close_logs();
  
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
