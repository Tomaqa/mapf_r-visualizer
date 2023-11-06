<div align="center">

# mapf_r-visualizer

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE.md)

Multi-agent pathfinding with continuous time (MAPF_R) visualizer
for research and educational usage.

</div>

Forked from [mapf-visualizer](https://github.com/Kei18/mapf-visualizer),
based on [openFrameworks](https://openframeworks.cc/).

Written in C++, developed for Linux (Arch Linux, Debian, Ubuntu).

<!-- ## Demo

![room-32-32-4](./assets/demo_room.gif)

![tunnel, planning with four agents](./assets/demo_tunnel.gif)

![ost003d, planning with 1000 agents](./assets/demo_ost003d.gif) -->

## Install

```sh
git clone --recurse-submodules https://github.com/Tomaqa/mapf_r-visualizer.git
cd mapf_r-visualizer
./install_linux.sh <distro>
make
```
where `<distro>` is one of `archlinux`, `debian`, `ubuntu`.

Required: around 10 minutes

## Usage

```sh
bin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l data/plan/sample.p
```

You can manipulate it via your keyboard. See printed info.

When the file with plan is omitted, such as
```sh
bin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l
```
then internal solver is invoked (which can take long time).


If a plan is not available for a pair of graph and layout,
you can pre-generate it.
It is not supported directly by this tool though,
you have to use my solver.
First, you must build it:
```sh
cd third_party/mapf_r
make
```
Then, to generate plan for a graph `<graph>` and a layout `<layout>`
and to store the plan into output file `<plan>`,
run
```sh
bin/release/mapf_r -g <graph> -l <layout> -p <plan>
```
(still within the directory `third_party/mapf_r`).
For example:
```sh
bin/release/mapf_r -g data/graph/sample.g -l data/layout/sample.l -p sample.p
```
The output file should use the extension `.p`

<!-- ## Input format of planning result

e.g.,
```txt
0:(5,16),(21,29),[...]
1:(5,17),(21,28),[...]
[...]
```

`(x, y)` denotes location.
`(0, 0)` is the left-top point.
`(x, 0)` is the location at `x`-th column and 1st row. -->

## Notes

- Error handling is poor
<!-- - The grid maps in `assets/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html) -->

## License

This software is released under the MIT License, see [LICENSE](LICENSE.md).

<!-- ## Author

[Keisuke Okumura](https://kei18.github.io) is a Ph.D. student at Tokyo Institute of Technology, interested in controlling multiple moving agents. -->
