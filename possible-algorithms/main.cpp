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


// Second Algorithm

function lookup(path):
  // Check whiteout first
  if whiteouts.contains(path): return NOT_FOUND

  // Cached?
  if cache.has(0, path): return cache.get(0, path)

  // Scan layers top -> bottom
  for i in range(0..L-1):
    if cache.has(i, path): return cache.get(i, path)
    meta = layers[i].stat(path)
    if meta.exists:
      cache.put(i, path, meta)
      return meta
  return NOT_FOUND

function write(path, data):
  meta = lookup(path)
  if meta == NOT_FOUND:
    // Creating new file in upper
    layers[0].create(path)
    cache.invalidate(path)
    return layers[0].write(path, data)

  if meta.layer_id == 0:
    return layers[0].write(path, data)

  // Need copy-up from lower layer
  src = layers[meta.layer_id].open(path, READ)
  layers[0].create(path)
  dst = layers[0].open(path, WRITE)
  copy(src, dst)         // O(size(file))
  cache.invalidate(path)
  return layers[0].write(path, data)
