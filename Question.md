## Scheduler

**Question:** What is the advantage of the scheduling strategy used in this assignment in comparison with other scheduling algorithms you have learned?

**Answer:** 
- Trong bài tập lớn này, việc sử dụng **priority queue** mang lại nhiều ưu điểm so với các thuật toán định thời khác (FCFS, SJR hay RR). Đầu tiên, **priority queue** dùng để quản lý các process dựa trên độ ưu tiên của chúng. Khi một process mới được thêm vào hệ thống, nó được đưa vào **priority queue** dựa trên độ ưu tiên của nó so với các process khác đang chờ. Điều này cho phép các process quan trọng được ưu tiên hơn các process ít quan trọng hơn, dẫn đến hiệu suất và khả năng đáp ứng của hệ thống tốt hơn.
- Bên cạnh đó, việc sử dụng **priority queue** cho phép định thời trong môi trường ưu tiên (preemptive scheduling). Nói cách khác, khi một process mới có độ ưu tiên cao hơn được thêm vào, nó có thể ngắt bỏ hoặc tạm dừng process có độ ưu tiên thấp hơn đang chạy để định thời process mới bất cứ lúc nào. Điều này giúp công việc định thời được đảm bảo rằng các process quan trọng, có độ ưu tiên cao được hoàn thành đúng thời hạn và không lãng phí các tài nguyên cho các process có độ ưu tiên thấp hơn.


## Memory Management

**Question:** In this simple OS, we implement a design of multiple memory segments or memory areas in source code declaration. What is the advantage of the proposed design of multiple segments?

**Question:** What will happen if we divide the address to more than 2-levels in the paging memory management system?

**Question:** What is the advantage and disadvantage of segmentation with paging?

**Question:** What will happen if the multi-core system has each CPU core can be run in a different context, and each core has its own MMU and its part of the core (the TLB)? In modern CPU, 2-level TLBs are common now, what is the impact of these new memory hardware configurations to our translation schemes?

## Put It All Together

**Question:** What will happen if the synchronization is not handled in your simple OS? Illustrate the problem of your simple OS by example if you have any. Note: You need to run two versions of your simple OS: the program with/without synchronization, then observe their performance based on demo results and explain their differences.
