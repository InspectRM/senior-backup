// pseudo code for DWFS Algorithm

initialize runqueue = empty min-heap (key = vruntime)
for each container i:
    set weight w_i (e.g. 1024 default), vruntime_i = 0
    insert container i into runqueue

loop forever:
    c = runqueue.pop_min()        // container with smallest vruntime
    run c on CPU for time Δ (fixed quantum or until preempt)
    cpu_time[c] += Δ
    vruntime[c] += Δ / w_c      // update weighted runtime
    // Optional: dynamic adjustment
    if (periodic epoch):
        for each container j:
            actual_j = cpu_time[j] / epoch_time
            ideal_j  = w_j / sum_of_weights
            if (actual_j > ideal_j + ε) then w_j := w_j * β_down
            else if (actual_j < ideal_j - ε) then w_j := w_j * β_up
        normalize weights (optional)
    update scheduling priority of c (based on new vruntime)
    runqueue.insert(c)
