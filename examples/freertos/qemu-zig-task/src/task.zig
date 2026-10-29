//! A FreeRTOS task written in Zig (freestanding). Same job as qemu-rust-task: statistics over samples from a C
//! producer, result sent back over a queue. Kernel calls go through the C shims in rtos_shim.c.
const Stats = extern struct { count: u32, min: i32, max: i32, sum: i32 };

extern fn rtos_queue_receive(q: ?*anyopaque, item: *anyopaque, wait_ms: u32) c_int;
extern fn rtos_queue_send(q: ?*anyopaque, item: *const anyopaque, wait_ms: u32) c_int;
extern fn rtos_log(s: [*:0]const u8) void;
extern fn rtos_suspend_self() void;

export fn zig_stats_task(params: ?*anyopaque) callconv(.c) void {
    const queues: *const [2]?*anyopaque = @ptrCast(@alignCast(params.?));
    rtos_log("zig task: running\n");
    var st = Stats{ .count = 0, .min = std_max_i32, .max = std_min_i32, .sum = 0 };
    while (true) {
        var sample: i32 = 0;
        if (rtos_queue_receive(queues[0], &sample, 500) == 0) break;
        if (sample == std_min_i32) break; // end marker
        st.count += 1;
        st.min = @min(st.min, sample);
        st.max = @max(st.max, sample);
        st.sum +%= sample;
    }
    _ = rtos_queue_send(queues[1], &st, 100);
    rtos_suspend_self();
}

const std_max_i32: i32 = 2147483647;
const std_min_i32: i32 = -2147483648;
