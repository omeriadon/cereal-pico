struct IntervalTask {
  let intervalSec: UInt32
  var lastRun: UInt32 = 0

  mutating func shouldRun(now: UInt32) -> Bool {
    if now - lastRun >= intervalSec * 1_000_000 {
      lastRun = now
      return true
    }
    return false
  }
}
