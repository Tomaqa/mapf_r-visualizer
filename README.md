<div align="center">

# mapf_r-visualizer

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE.md)

Multi-agent pathfinding with continuous time (MAPF_R) visualizer
for research and educational usage.

</div>

Forked from [mapf-visualizer](https://github.com/Kei18/mapf-visualizer),
based on [openFrameworks](https://openframeworks.cc/).

Written in C++, developed for Linux (Arch Linux, Debian, Ubuntu).

## Demo

![grid_04x04_k3_k4_D](doc/img/gif/grid/grid_04x04_k3_k4_D_s005.gif)
![grid_04x04_k3_k4_D_2smaller](doc/img/gif/grid/grid_04x04_k3_k4_D_2smaller_s005.gif)

![grid_04x04_k3_k4_E](doc/img/gif/grid/grid_04x04_k3_k4_E_s005.gif)

![grid_04x04_k3_k6_D](doc/img/gif/grid/grid_04x04_k3_k6_D_s01.gif)
![grid_04x04_k3_k6_E](doc/img/gif/grid/grid_04x04_k3_k6_E_s01.gif)

![empty-16-16-random_n3-1_k30_s01](doc/img/gif/empty/empty-16-16-random_n3-1_k30_s01.gif)
![empty-16-16-random_n3-5_k50_s01](doc/img/gif/empty/empty-16-16-random_n3-5_k50_s01.gif)

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
