## Scheduler

**Question:** What is the advantage of the scheduling strategy used in this assignment in comparison with other scheduling algorithms you have learned?

**Answer:** 
- Trong bài tập lớn này, việc sử dụng **priority queue** mang lại nhiều ưu điểm so với các thuật toán định thời khác (FCFS, SJR hay RR). Đầu tiên, **priority queue** dùng để quản lý các process dựa trên độ ưu tiên của chúng. Khi một process mới được thêm vào hệ thống, nó được đưa vào **priority queue** dựa trên độ ưu tiên của nó so với các process khác đang chờ. Điều này cho phép các process quan trọng được ưu tiên hơn các process ít quan trọng hơn, dẫn đến hiệu suất và khả năng đáp ứng của hệ thống tốt hơn.
- Bên cạnh đó, việc sử dụng **priority queue** cho phép định thời trong môi trường ưu tiên (preemptive scheduling). Nói cách khác, khi một process mới có độ ưu tiên cao hơn được thêm vào, nó có thể ngắt bỏ hoặc tạm dừng process có độ ưu tiên thấp hơn đang chạy để định thời process mới bất cứ lúc nào. Điều này giúp công việc định thời được đảm bảo rằng các process quan trọng, có độ ưu tiên cao được hoàn thành đúng thời hạn và không lãng phí các tài nguyên cho các process có độ ưu tiên thấp hơn.
