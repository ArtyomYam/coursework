#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp> 
#include<chrono>


struct Point {
    int x, y;
};


struct Edge {
    Point A_;
    Point B_;

    bool verification = true;

    Edge(Point A, Point B) {
        A_ = A;
        B_ = B;
    }

    double length() {
        return sqrt(pow(A_.x - B_.x, 2) + pow(A_.y - B_.y, 2));
    }

   //метод проверки на пересечение с другим отрезком на основе векторного прооизведения
    bool crosses(Edge rhs) {
        Point crossing;
        double d = (A_.x - B_.x) * (rhs.B_.y - rhs.A_.y) - (A_.y - B_.y) * (rhs.B_.x - rhs.A_.x);
        if (d == 0) {
            return false;
        }
        double ua = ((rhs.A_.x - rhs.B_.x) * (A_.y - rhs.A_.y) - (rhs.A_.y - rhs.B_.y) * (A_.x - rhs.A_.x)) / d;
        double ub = ((A_.x - B_.x) * (A_.y - rhs.A_.y) - (A_.y - B_.y) * (A_.x - rhs.A_.x)) / d;
        if (ua < 0 || ua > 1 || ub < 0 || ub > 1) {
            return false;
        }
        crossing.x = int(A_.x + ua * (B_.x - A_.x));
        crossing.y = int(A_.y + ua * (B_.y - A_.y));
        if (crossing.x == rhs.A_.x && crossing.y == rhs.A_.y || crossing.x == rhs.B_.x && crossing.y == rhs.B_.y) {
            return false;
        }
        return true;
    }
};



//функция создания триангуляции (списка валидных и невалидных отрезков)
std::vector<Edge> triangulate(std::vector<Point> points) {
    std::vector<Edge> edges;
    std::vector<Edge> result;
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = i + 1; j < points.size(); j++) {
            edges.push_back(Edge(points[i], points[j]));
        }
    }

    // Cортируем ребра по длине по возрастанию
    for (int i = 0; i < edges.size() - 1; i++) {
        for (int j = 0; j < edges.size() - i - 1; j++) {
            if (edges[j].length() > edges[j + 1].length()) {
                // меняем элементы местами
                Edge box = edges[j];
                edges[j] = edges[j + 1];
                edges[j + 1] = box;
            }
        }
    }

    //отсортированные рёбра проверяем на пересечение с предыдущими
    for (int i = 0; i < edges.size(); i++) {
        for (int j = 0; j < result.size(); j++) {
            // тут проверить на пересечение со всеми текущими подходящими ребрами, и если вдруг пересекает -> Edge.verification = false;
            if (edges[i].crosses(result[j])) {
                edges[i].verification = false;
                break;
            }
        }
        result.push_back(edges[i]);
    }
    return result;
}

//void onWindowClose(int event, void* userdata) {
//    if (event == cv::EVENT_LBUTTONUP) {
//        cv::destroyAllWindows();
//    }
//}

bool isWindowClosed(const std::string& windowName) {
    int autosize = cv::getWindowProperty(windowName, cv::WND_PROP_AUTOSIZE);
    return autosize == -1;
}

int main() {
    //список точек для отрисовки триангуляции
    std::vector<Point> points = { {80, 720}, {700, 500}, {900, 740}, {250, 500}, {750, 600}, {700, 100}};
    //создаём список всех возмоных отрезков с указанием, пересекают ли они соседние
    std::vector<Edge> result = triangulate(points);
    //список уже отрисованных отрезков
    std::vector<Edge> drawen;
    
    cv::Mat final_image(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat image_green(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat image_red(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));


    //отрисовываем исходные точки
    for (int i = 0; i < points.size(); i++) {
        cv::Point centre(points[i].x, points[i].y);
        cv::circle(image_green, centre, 2, cv::Scalar(255, 255, 255), 4);
        cv::circle(final_image, centre, 2, cv::Scalar(255, 255, 255), 4);
    }


    //Создаём окно для работы
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    for (int i = 0; i < result.size(); i++) {
//        std::cout << i << ": (" << result[i].A_.x << ", " << result[i].A_.y << ")--(" << result[i].B_.x << ", " << result[i].B_.y << ")" << std::endl;
        if (result[i].verification == true){
            line(image_green, cv::Point(result[i].A_.x, result[i].A_.y), cv::Point(result[i].B_.x, result[i].B_.y), cv::Scalar(0, 255, 255), 4);
            line(final_image, cv::Point(result[i].A_.x, result[i].A_.y), cv::Point(result[i].B_.x, result[i].B_.y), cv::Scalar(0, 255, 255), 4);
            drawen.push_back(result[i]);
            cv::imshow("Display window", image_green);
//            cv::setMouseCallback("Display window", onWindowClose(NULL));
//            cv::waitKey(1000);
            int k = cv::waitKey(1000);
            if (k == 27) {
                break;
            }
            //if (isWindowClosed("Display window")) {
            //    cv::destroyAllWindows();
            //    break;
            //}
        }
        else {
            cv::Mat image_red(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));
            for (int k = 0; k < drawen.size(); k++) {
                line(image_red, cv::Point(drawen[k].A_.x, drawen[k].A_.y), cv::Point(drawen[k].B_.x, drawen[k].B_.y), cv::Scalar(0, 255, 255), 4);
            }
            line(image_red, cv::Point(result[i].A_.x, result[i].A_.y), cv::Point(result[i].B_.x, result[i].B_.y), cv::Scalar(0, 0, 255), 4);
            cv::imshow("Display window", image_red);
            int k = cv::waitKey(1000);
            if (k == 27) {
                break;
            }
            //if (isWindowClosed("Display window")) {
            //    cv::destroyAllWindows();
            //    break;
            //}
        }
    }

  //  cv::imshow("Display window", final_image);
  //  cv::waitKey(0);
    return 0;
}
