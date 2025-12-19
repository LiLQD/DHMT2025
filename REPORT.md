# BÁO CÁO BÀI TẬP LỚN ĐỒ HỌA MÁY TÍNH
## ĐỀ TÀI: MÔ PHỎNG GIAN HÀNG BÁN MÁY BAY ĐỒ CHƠI TRẺ EM

### 1. Giới thiệu đề tài
Dự án mô phỏng một gian hàng trưng bày các loại máy bay đồ chơi với các hiệu ứng chuyển động và ánh sáng 3D sử dụng thư viện OpenGL và ngôn ngữ C++. Ứng dụng cho phép người dùng tham quan gian hàng và điều khiển từng loại máy bay với các hành vi mô phỏng riêng biệt.

### 2. Kỹ thuật sử dụng
- **OpenGL Programmable Pipeline**: Sử dụng Core Profile (bắt buộc shader).
- **GLSL Shaders**:
  - Vertex Shader: Tính toán chiếu sáng theo mô hình Gouraud/Phong (tùy chỉnh), biến đổi MVP (Model-View-Projection).
  - Fragment Shader: Nội suy màu sắc.
- **Mô hình phân cấp (Hierarchical Modeling)**:
  - Cấu trúc cây node cho các đối tượng phức tạp như Trực thăng (Thân -> Rotor Chính, Rotor Đuôi), Drone (Thân -> 4 Cánh quạt), v.v.
  - Sử dụng Stack ma trận (hoặc nhân ma trận đệ quy) để quản lý trạng thái biến đổi.
- **Chiếu sáng (Lighting)**:
  - Mô hình Blinn-Phong (Ambient, Diffuse, Specular).
  - Ánh sáng nguồn điểm (Point Light) và tùy chỉnh vật liệu (Material Properties).

### 3. Cấu trúc chương trình
- **main.cpp**: Chứa vòng lặp chính, xử lý logic vẽ, và input.
- **Shaders**: `vshader.glsl` và `fshader.glsl` xử lý đồ họa trên GPU.
- **Geometry**: Các hàm sinh tọa độ cho hình lập phương (`generateCube`) và hình trụ (`generateCylinder`) sử dụng mảng đỉnh (Vertex Arrays).

### 4. Danh sách các đối tượng mô phỏng
1. **Máy bay phản lực**: Có động cơ xoay, bánh đáp thu/mở.
2. **Máy bay cánh quạt**: Cánh quạt quay liên tục quanh trục Z.
3. **Trực thăng**: Cánh quạt chính và cánh quạt đuôi quay độc lập.
4. **Máy bay giấy**: Mô phỏng hình dáng đơn giản.
5. **Drone**: 4 cánh quạt quay, thân nhỏ gọn.
6. **Tên lửa**: Dáng khí động học, có thể phóng thẳng đứng.
7. **Khinh khí cầu**: Mô phỏng bay lơ lửng.
8. **Tiêm kích**: Cánh rộng, trang bị tên lửa giả lập.

### 5. Hướng dẫn sử dụng
- Chạy chương trình, cửa sổ OpenGL hiện ra với gian hàng.
- Nhấn **0** để về chế độ Camera tự do (Free Cam).
- Nhấn số **1-8** để chọn máy bay tương ứng.
- Phím điều khiển: **W/S/A/D** (Di chuyển), **Q/E** (Lên/Xuống).
- Phím chức năng riêng: **U/J** (Quay cánh quạt/Tốc độ).

### 6. Kết luận
Dự án đã đáp ứng đầy đủ các yêu cầu về kỹ thuật OpenGL hiện đại, kỹ thuật mô hình hóa phân cấp và logic điều khiển tương tác thời gian thực.
