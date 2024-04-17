# <img src="https://upload.wikimedia.org/wikipedia/commons/f/f0/HCMCUT.svg" alt="HCMUT" width="23" /> Operating System: Assignment

## Source code

- Header file

  - `timer.h`: Xác định bộ đếm thời gian cho toàn bộ hệ thống.
  - `cpu.h`: Xác định các chức năng được sử dụng để triển khai CPU ảo.
  - `queue.h`: Các hàm được sử dụng để triển khai hàng đợi chứa PCB của các tiến trình.
  - `sched.h`: Xác định các chức năng được sử dụng bởi bộ định thời.
  - `mem.h`: Các chức năng được Virtual Memory Engine sử dụng.
  - `loader.h`: Các hàm được trình tải sử dụng để tải chương trình từ đĩa vào bộ nhớ
  - `common.h`: Xác định các cấu trúc và hàm được sử dụng ở mọi nơi trong HĐH.
  - `bitopts.h`: Xác định các thao tác trên dữ liệu bit.
  - `os-mm.h`, `mm.h`: Xác định cấu trúc và dữ liệu cơ bản cho Quản lý bộ nhớ dựa trên phân trang.
  - `os-cfg.h`: (Tùy chọn) Xác định các hằng số sử dụng để chuyển đổi cấu hình phần mềm.

- Source file

  - `timer.c`: Thực hiện bộ đếm thời gian.
  - `cpu.c`: Triển khai CPU ảo.
  - `queue.c`: Thực hiện các thao tác trên hàng đợi (ưu tiên).
  - `paging.c`: Sử dụng để kiểm tra chức năng của Virtual Memory Engine.
  - `os.c`: Toàn bộ hệ điều hành bắt đầu chạy từ file này.
  - `loader.c`: Triển khai trình tải.
  - `sched.c`: Triển khai bộ định thời.
  - `mem.c`: Triển khai RAM và Virtual Memory Engine.
  - `mm.c`, `mm-vm.c`, `mm-memphy.c`: Triển khai quản lý bộ nhớ dựa trên phân trang.

## Requirments

- **Định thời:** 
