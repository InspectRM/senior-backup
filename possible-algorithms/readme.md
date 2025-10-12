# Example Use-Case: 

Consider two containers A and B on a single-core system, with initial weights 2 and 

`1. DWFS initially sets vruntime_A = vruntime_B = 0.`

On the first scheduling round, assume A is picked (tie broken arbitrarily). Suppose A runs for 10ms; then vruntime_A += 10/2 = 5, while vruntime_B stays 0. 

Next, B has the smaller vruntime (0 vs. 5), so B runs. If B runs 10ms, vruntime_B += 10/1 = 10. Now vruntime_A=5, vruntime_B=10, so A runs again. The pattern (A→B→A→…) yields A getting roughly twice the CPU of B. 

If A ran too long (say 8ms instead of 5), its vruntime would grow more, possibly delaying it in the queue; the epoch adjustment would then slightly lower A’s weight to compensate. In a larger scenario with many containers, DWFS’s queueing ensures each container’s share matches its weight proportionally. This example demonstrates how DWFS maintains fairness and adjusts priorities to keep the CPU distribution in line with the configured weights.

# Complexity Analysis: Let n be the number of containers. 

DWFS keeps containers in a priority queue (e.g. a balanced tree or heap) sorted by vruntime. Insertion or update in a red-black tree (as in Linux CFS) is O(log n), and selecting the next container is O(1) if the minimum is cached. If using a binary heap, both extract-min and insert are O(log n). Updating vruntime and weights is O(1) per container. Thus each scheduling decision is O(log n) time (dominated by queue operations), and overall space is O(n) to store container states. Tracking CPU usage by reading /proc or cgroup stats adds an O(n) scan (once per interval), still O(n) total. Hence DWFS is efficient: in practice for typical n the overhead is low and matches known fair schedulers. 