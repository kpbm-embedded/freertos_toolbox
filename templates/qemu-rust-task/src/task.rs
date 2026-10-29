//! A FreeRTOS task written in Rust (no_std). It receives sensor samples from a C producer task over a queue,
//! keeps a running statistic and sends results back. Kernel calls go through the C shims in rtos_shim.c.
#![no_std]

use core::ffi::c_void;
use core::panic::PanicInfo;

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct Stats { pub count: u32, pub min: i32, pub max: i32, pub sum: i32 }

extern "C" {
    fn rtos_queue_receive(q: *mut c_void, item: *mut c_void, wait_ms: u32) -> i32;
    fn rtos_queue_send(q: *mut c_void, item: *const c_void, wait_ms: u32) -> i32;
    fn rtos_log(s: *const u8);
    fn rtos_suspend_self();
}

/// Task entry: pvParameters points at two queue handles [samples_in, stats_out].
#[no_mangle]
pub extern "C" fn rust_stats_task(params: *mut c_void) {
    let queues = unsafe { &*(params as *const [*mut c_void; 2]) };
    unsafe { rtos_log(b"rust task: running\n\0".as_ptr()) };
    let mut st = Stats { count: 0, min: i32::MAX, max: i32::MIN, sum: 0 };
    loop {
        let mut sample: i32 = 0;
        if unsafe { rtos_queue_receive(queues[0], &mut sample as *mut i32 as *mut c_void, 500) } == 0 {
            break;
        }
        if sample == i32::MIN { break; }               // end marker
        st.count += 1;
        st.min = st.min.min(sample);
        st.max = st.max.max(sample);
        st.sum = st.sum.wrapping_add(sample);
    }
    unsafe {
        rtos_queue_send(queues[1], &st as *const Stats as *const c_void, 100);
        rtos_suspend_self();
    }
}

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    unsafe { rtos_log(b"rust panic\nRESULT: FAIL (panic)\n\0".as_ptr()) };
    loop {}
}
