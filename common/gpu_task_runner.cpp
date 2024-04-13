#include "gpu_task_runner.h"
#include "core/os/memory.h"
#include <memory>
#include <utility>

namespace pgvoxel {

GPUTaskRunner::GPUTaskRunner() {}

GPUTaskRunner::~GPUTaskRunner() {
	stop();
}

void GPUTaskRunner::start(RenderingDevice *rd, GPUStorageBufferPool *pool) {
	_rendering_device = rd;
	_storage_buffer_pool = pool;
	_running = true;
	_thread.start(
			[](void *p_userdata) {
				GPUTaskRunner *runner = static_cast<GPUTaskRunner *>(p_userdata);
				runner->thread_func();
			},
			this);
}

void GPUTaskRunner::stop() {
	_running = false;
	_semaphore.post();
	_thread.wait_to_finish();
}

void GPUTaskRunner::push(std::unique_ptr<IGPUTask> task) {
	MutexLock mlock(_mutex);
	_shared_tasks.push_back(std::move(task));
	_semaphore.post();
	++_pending_count;
}

unsigned int GPUTaskRunner::get_pending_task_count() const {
	return _pending_count;
}

void GPUTaskRunner::thread_func() {
	std::vector<std::unique_ptr<IGPUTask>> tasks;

	// We use a common output buffer for tasks that need to download results back to the CPU,
	// because a single call to `buffer_get_data` is cheaper than multiple ones, due to Godot's API being synchronous.
	// 使用一个共享 buffer 来从gpu中取数据，这样比多次调用 buffer_get_data 更快，因为 Godot 的API不是异步的
	RID shared_output_storage_buffer_rid;
	unsigned int shared_output_storage_buffer_capacity = 0;
	std::vector<std::pair<unsigned, unsigned>> shared_output_storage_buffer_segments;

	// Godot does not support async compute, so in order to get results from a compute shader, the only way is to sync
	// with the device, waiting for everything to complete. So instead of running one shader at a time, we run a few of
	// them.
	// It's also unclear how much to execute per frame.
	// 4 tasks was good enough on an nVidia 1060 for detail rendering, but for tasks with different costs it might need
	// different quota to prevent rendering slowdowns...
	// Godot 不支持异步计算，所以为了从计算着色器中获取结果，唯一的方法是等待设备完成所有工作。因此一次必须运行多个任务，而不是一个一个来。
	// 目前还不清楚每个帧应该运行多少个任务。
	// 在 NVIDIA 1060 上，4 个任务就足够完成渲染，但是对于计算量不同的任务，可能需要不同的配额来防止拖慢渲染速度...
	const unsigned int batch_count = 16;

	while (_running) {
		while (_shared_tasks.size() == 0) {
			_semaphore.wait();
		}
		{
			MutexLock mlock(_mutex);
			tasks = std::move(_shared_tasks);
		}

		GPUTaskContext ctx(*_rendering_device, *_storage_buffer_pool);

		for (size_t begin_index = 0; begin_index < tasks.size(); begin_index += batch_count) {
			const size_t end_index = std::min(begin_index + batch_count, tasks.size());

			unsigned int required_shared_output_buffer_size = 0;
			shared_output_storage_buffer_segments.clear();

			// Get how much data we'll want to download from the GPU for this batch
			// 获取当前批次的需要从gpu中下载的数据量
			for (size_t i = begin_index; i < end_index; ++i) {
				IGPUTask *task = tasks[i].get();
				const unsigned size = task->get_required_shared_output_buffer_size();
				// TODO Should we pad sections with some kind of alignment?
				// TODO: 或许可以考虑在对齐时进行填充
				shared_output_storage_buffer_segments.push_back({ required_shared_output_buffer_size, size });
				required_shared_output_buffer_size += size;
			}

			// Make sure we allocate a storage buffer that can contain all output data in this batch
			// 确保当前的共享buffer能存下当前 batch 的所有输出数据
			if (required_shared_output_buffer_size > shared_output_storage_buffer_capacity) {
				if (shared_output_storage_buffer_rid.is_valid()) {
					ctx.rendering_device.free(shared_output_storage_buffer_rid);
				}
				// TODO Resize to some multiplier above?
				// TODO: 或许可以使用倍增的方式扩容
				shared_output_storage_buffer_rid =
						ctx.rendering_device.storage_buffer_create(required_shared_output_buffer_size);
				shared_output_storage_buffer_capacity = required_shared_output_buffer_size;
			}

			ctx.shared_output_buffer_rid = shared_output_storage_buffer_rid;

			// Prepare tasks
			// 准备任务
			for (size_t i = begin_index; i < end_index; ++i) {
				const auto [position, size] = shared_output_storage_buffer_segments[i - begin_index];
				ctx.shared_output_buffer_begin = position;
				ctx.shared_output_buffer_size = size;

				IGPUTask *task = tasks[i].get();
				task->prepare(ctx);
			}

			// Submit work and wait for completion
			// 提交任务并等待计算完成
			ctx.rendering_device.submit();
			ctx.rendering_device.sync();

			// Download data from shared buffer
			// 从共享buffer中下载数据
			if (required_shared_output_buffer_size > 0 && shared_output_storage_buffer_rid.is_valid()) {
				// Unfortunately we can't re-use memory for that buffer, Godot will always want to allocate it using
				// malloc. That buffer can be a few megabytes long...
				// 很不幸，无法重用这个buffer的内存，因为 Godot 总是会用 malloc 分配一块新内存给返回值，该 buffer 可能会占用几 MB
				ctx.downloaded_shared_output_data = ctx.rendering_device.buffer_get_data(
						shared_output_storage_buffer_rid, 0, required_shared_output_buffer_size);
			}

			// Collect results and complete tasks
			// 收集结果并完成任务
			for (size_t i = begin_index; i < end_index; ++i) {
				const auto [position, size] = shared_output_storage_buffer_segments[i - begin_index];
				ctx.shared_output_buffer_begin = position;
				ctx.shared_output_buffer_size = size;

				IGPUTask *task = tasks[i].get();
				task->collect(ctx);
				memdelete(task);
				--_pending_count;
			}

			ctx.downloaded_shared_output_data = PackedByteArray();
		}

		tasks.clear();
	}

	if (shared_output_storage_buffer_rid.is_valid()) {
		_rendering_device->free(shared_output_storage_buffer_rid);
	}
}

} //namespace pgvoxel
