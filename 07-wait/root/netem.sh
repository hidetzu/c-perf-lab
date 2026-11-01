#!/bin/bash
# 第9章の「往復遅延」と「名前解決」を、tc netem で lo に遅延を入れて測る。root が必要。使い方: sudo -v のあと bash netem.sh [遅延ms (既定 2)]
# 注意: lo の遅延は、この間 127.0.0.0/8 宛の通信すべて (systemd-resolved のスタブ 127.0.0.53 を含む) に効く。終了時 (Ctrl-C でも) に必ず元に戻す。
set -e; D=${1:-2}; ROOT=$(cd "$(dirname "$0")/../.." && pwd); R=$ROOT/tools/runstat; W=$ROOT/07-wait
for b in "$R" "$W/w3" "$W/w4" "$W/w5"; do [ -x "$b" ] || { echo "$b がない。bash build_all.sh を先に" >&2; exit 2; }; done
cleanup(){ sudo tc qdisc del dev lo root 2>/dev/null || true; }
trap cleanup EXIT INT TERM
sudo tc qdisc del dev lo root 2>/dev/null || true
echo "## 遅延なし (基準)"
for m in seq pipeline; do printf "tcp %-9s 200件 " $m; $R - 3 2 $W/w3 $m 200 | sed 's/(median.*//'; done
for t in 1 8; do $W/w4 x.invalid 40 $t; done
for m in seq pipeline; do $W/w5 $m 50; done
sudo tc qdisc add dev lo root netem delay ${D}ms
echo "## lo に netem delay ${D}ms (往復で約 $((D*2)) ms)"; tc qdisc show dev lo
for m in seq pipeline; do printf "tcp %-9s 200件 " $m; $R - 3 2 $W/w3 $m 200 | sed 's/(median.*//'; done
for t in 1 8; do $W/w4 x.invalid 40 $t; done
$W/w4 localhost 40 1
for m in seq pipeline; do $W/w5 $m 50; done
echo "(w4 = getaddrinfo。この環境では nss-resolve が systemd-resolved と Unix ソケットで話すので lo の遅延が効かない。w5 = スタブリゾルバへ UDP で直接問い合わせるので lo の遅延が効く)"
cleanup; trap - EXIT; echo "## netem を外した: $(tc qdisc show dev lo | head -1)"
