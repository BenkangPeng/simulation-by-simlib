### Reference
The fold `simlib/` refered to [yusufRahmatullah](https://github.com/yusufRahmatullah/simlib)

And I had modified its source code of `simlib.c` to fit my needs:
```
line 557        if (variable < 0)
                {				/* Report summary statistics in transfer. */
                    ivar = -variable;
                    area[ivar] += (sim_time - tlvc[ivar]) * preval[ivar];
                    tlvc[ivar] = sim_time;
                    transfer[1] = area[ivar] / (sim_time - treset);
                    transfer[2] = max[ivar];
                    transfer[3] = min[ivar];
+                   transfer[4] = area[ivar];
                    return transfer[1];
line 567        }

```

### Usage
**for elevator simulation by simlib**
```shell
$ cd examples/elevator-system-model/
$ make run
```
Above command will generate the `esm.out` and `debug.log`, which is the report of simulation and debug information respectively.


**for elevator simulation by gpss world**
1. Firstly, you mush install the `examples/gpss-elevator-simulation/gpss_world_student_setup.msi` in your windows machine.
2. Secondly, use the software installed above to open the `examples/gpss-elevator-simulation/elevator.gps` file.
3. Lastly, it's your time to start simulation using this old and fancy gpss-world software. Maybe [gpss world tutorial](https://athena.ecs.csus.edu/~mitchell/csc148/gpssW/Tutorial%20Manual/tutorial_manual.htm) and [gpss world user reference](https://athena.ecs.csus.edu/~mitchell/csc148/gpssW/Reference%20Manual/reference_manual.htm) can help you.