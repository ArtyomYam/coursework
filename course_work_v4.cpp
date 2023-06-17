#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp> 
#include<chrono>
#include<fstream>


struct Point {
    double x_ = 0;
    double y_ = 0;

    //int x_ = 0;
    //int y_ = 0;

    Point() = default;
    ~Point() = default;

    Point(const Point& other) = default;
    Point& operator=(const Point& other) = default;

    Point(Point&& other) = default;
    Point& operator=(Point&& other) = default;

};


struct Edge {
    Point A_;
    Point B_;

    bool verification = true;

    Edge() = default;
    ~Edge() = default;

    Edge(const Edge& other) = default;
    Edge& operator=(const Edge& other) = default;

    Edge(Edge&& other) = default;
    Edge& operator=(Edge&& other) = default;

    Edge(Point A, Point B) {
        A_ = A;
        B_ = B;
    }

    double length() {
        return sqrt(pow(A_.x_ - B_.x_, 2) + pow(A_.y_ - B_.y_, 2));
    }

    //метод проверки на пересечение с другим отрезком на основе векторного прооизведения
    bool crosses(Edge rhs) {
        Point crossing;
        double d = (A_.x_ - B_.x_) * (rhs.B_.y_ - rhs.A_.y_) - (A_.y_ - B_.y_) * (rhs.B_.x_ - rhs.A_.x_);
        if (d == 0) {
            return false;
        }
        double ua = ((rhs.A_.x_ - rhs.B_.x_) * (A_.y_ - rhs.A_.y_) - (rhs.A_.y_ - rhs.B_.y_) * (A_.x_ - rhs.A_.x_)) / d;
        double ub = ((A_.x_ - B_.x_) * (A_.y_ - rhs.A_.y_) - (A_.y_ - B_.y_) * (A_.x_ - rhs.A_.x_)) / d;
        if (ua < 0 || ua > 1 || ub < 0 || ub > 1) {
            return false;
        }
        crossing.x_ = double(A_.x_ + ua * (B_.x_ - A_.x_));
        crossing.y_ = double(A_.y_ + ua * (B_.y_ - A_.y_));
        if (crossing.x_ == rhs.A_.x_ && crossing.y_ == rhs.A_.y_ || crossing.x_ == rhs.B_.x_ && crossing.y_ == rhs.B_.y_) {
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
            }
        }
        result.push_back(edges[i]);
    }
    return result;
}

void makePreamble(std::ofstream& fout) {
    fout << R"(\documentclass[a4paper]{article})" << std::endl;
    fout << R"(\usepackage{pgfplots})" << std::endl;
    fout << R"(\usepackage{tikz})" << std::endl;
    fout << R"(\usepackage{tkz-euclide})" << std::endl;
    fout << R"(\usepackage[english, russian]{babel})" << std::endl;
    fout << R"(\usepackage[T2A]{fontenc})" << std::endl;
    fout << R"(\usepackage[utf8]{inputenc})" << std::endl;
    fout << R"(\usepackage{geometry})" << std::endl;
    fout << R"(\pgfplotsset{compat = 1.18})" << std::endl;
    fout << R"(\geometry{top = 20mm})" << std::endl;
    fout << R"(\geometry{bottom = 25mm})" << std::endl;
    fout << R"(\geometry{left = 25mm})" << std::endl;
    fout << R"(\geometry{right = 25mm})" << std::endl;
    fout << R"(\title{Visualization of the Greedy triangulation algorithm.})" << std::endl;
    fout << R"(\begin{document})" << std::endl;
    fout << R"(\maketitle)" << std::endl;
}

void PointsRedrawing(const std::vector<Point>& points, std::ofstream& fout, const cv::Mat& input_image = cv::Mat()) {
    if (input_image.empty()) {
        for (int i = 0; i < points.size(); i++) {
            fout << R"(\filldraw[black])" << "(" << points[i].x_ / 50 << ","
                << -(points[i].y_ / 50) << ")" << "circle(2pt);" << std::endl;
        }
    }
    else {
        for (int i = 0; i < points.size(); i++) {
            fout << R"(\filldraw[black])" << "(" << points[i].x_ / 50 << ","
                << -(points[i].y_ / 50) << ")" << "circle(2pt);" << std::endl;
            cv::Point centre(static_cast<int>(points[i].x_), static_cast<int>(points[i].y_));
            cv::circle(input_image, centre, 2, cv::Scalar(255, 255, 255), 4);
        }
    }
}

void LineDrawing(std::ofstream& fout, int& k, std::string color, std::vector<Edge> list = {}) {
    fout << R"(\draw[ultra thick, )" << color << "](" << list[k].A_.x_ / 50 << ", "
        << -(list[k].A_.y_ / 50) << ")--" << "("
        << list[k].B_.x_ / 50 << ", "
        << -(list[k].B_.y_ / 50) << ");" << std::endl;
}



//функция проверяющая видимость окна
bool isWindowClosed(const std::string& windowName) {
    return cv::getWindowProperty(windowName, cv::WND_PROP_VISIBLE) < 1;
}


void triangulation(std::vector<Point> input) {
    //список точек для отрисовки триангуляции
    std::vector<Point> points = input;

    std::ofstream fout; // Создание файла, запись кода LaTex и визуализация
    fout.open("visualization.txt", std::ofstream::out | std::ofstream::trunc);
    makePreamble(fout);
    // переменная для определения границ изображения
    bool closed = false;
    fout << R"(\section{Points layout})" << std::endl;
    fout << R"(\begin{tikzpicture})" << std::endl;


    //создаём список всех возмоных отрезков с указанием, пересекают ли они соседние
    std::vector<Edge> result = triangulate(points);

    //список уже отрисованных отрезков
    std::vector<Edge> drawen;

    cv::Mat image_green(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat image_red(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));


    //отрисовываем исходные точки
    for (int i = 0; i < points.size(); i++) {
        cv::Point centre(static_cast<int>(points[i].x_), static_cast<int>(points[i].y_));
        //cv::Point centre(points[i].x_, points[i].y_);
        cv::circle(image_green, centre, 2, cv::Scalar(255, 255, 255), 4);
        fout << R"(\filldraw[red])" << "(" << points[i].x_ / 50 << ","
            << -(points[i].y_ / 50) << ")" << "circle(4pt);" << std::endl;
    }
    fout << R"(\end{tikzpicture})" << std::endl;
    closed = true;

    //Создаём окно для работы
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    //флаг показывающий заврешилась ли программа штатно и нужно ли выводить итоговый результат
    bool finished = true;

    for (int i = 0; i < result.size(); i++) {
        if (result[i].verification == true) {
            cv::Point point1(static_cast<int>(result[i].A_.x_), static_cast<int>(result[i].A_.y_));
            cv::Point point2(static_cast<int>(result[i].B_.x_), static_cast<int>(result[i].B_.y_));
            //line(image_green, cv::Point(result[i].A_.x_, result[i].A_.y_), cv::Point(result[i].B_.x_, result[i].B_.y_), cv::Scalar(0, 255, 255), 4);
            line(image_green, point1, point2, cv::Scalar(0, 255, 255), 4);

            if (closed) {
                fout << R"(\section{ood edges})" << std::endl;
                fout << R"(\begin{tikzpicture})" << std::endl;
                closed = false;
                PointsRedrawing(points, fout);
            }

            for (int k = 0; k < drawen.size(); k++) {
                LineDrawing(fout, k, "green", drawen);
            }

            LineDrawing(fout, i, "green", result);

            drawen.push_back(result[i]);
            cv::imshow("Display window", image_green);
            int k = cv::waitKey(1000);
            if (k == 27 || isWindowClosed("Display window")) {
                finished = false;
                break;
                cv::destroyAllWindows();
            }
        }
        else {
            if (!closed) {
                fout << R"(\end{tikzpicture})" << std::endl;
                closed = true;
            }
            fout << R"(\section{Adding bad edges.})" << std::endl;
            fout << R"(\begin{tikzpicture})" << std::endl;
            closed = false;

            cv::Mat image_red(800, 1450, CV_8UC3, cv::Scalar(0, 0, 0));

            PointsRedrawing(points, fout, image_red);

            for (int k = 0; k < drawen.size(); k++) {
                cv::Point point1(static_cast<int>(drawen[k].A_.x_), static_cast<int>(drawen[k].A_.y_));
                cv::Point point2(static_cast<int>(drawen[k].B_.x_), static_cast<int>(drawen[k].B_.y_));
                //line(image_red, cv::Point(drawen[k].A_.x_, drawen[k].A_.y_), cv::Point(drawen[k].B_.x_, drawen[k].B_.y_), cv::Scalar(0, 255, 255), 4);
                line(image_red,point1 ,point2 , cv::Scalar(0, 255, 255), 4);
                LineDrawing(fout, k, "green", drawen);
            }
            
            cv::Point point1(static_cast<int>(result[i].A_.x_), static_cast<int>(result[i].A_.y_));
            cv::Point point2(static_cast<int>(result[i].B_.x_), static_cast<int>(result[i].B_.y_));
            //line(image_red, cv::Point(result[i].A_.x_, result[i].A_.y_), cv::Point(result[i].B_.x_, result[i].B_.y_), cv::Scalar(0, 0, 255), 4);
            line(image_red,point1 , point2, cv::Scalar(0, 0, 255), 4);

            LineDrawing(fout, i, "red", result);
            fout << R"(\end{tikzpicture})" << std::endl;
            closed = true;

            cv::imshow("Display window", image_red);
            int k = cv::waitKey(1000);
            if (k == 27 || isWindowClosed("Display window")) {
                finished = false;
                break;
                cv::destroyAllWindows();
            }
        }
    }
    if (finished == true) {
        cv::imshow("Display window", image_green);
        cv::waitKey(0);
    }

    fout << R"(\section{Final result.})" << std::endl;
    fout << R"(\begin{tikzpicture})" << std::endl;
    PointsRedrawing(points, fout);
    for (int k = 0; k < drawen.size(); k++) {
        LineDrawing(fout, k, "green", drawen);
    }
    fout << R"(\end{tikzpicture})" << std::endl;
    fout << R"(\end{document})" << std::endl;

//    std::system("pdflatex visualisation.tex");
}


int main() {
    std::string filename = "points.txt";
    std::ifstream input_file;
    std::string path = "../../../";
    input_file.open(path + filename);
    std::vector<Point> points;

    if (!input_file.is_open()) { // если файл не открыт
        std::cout << "error\n"; // сообщить об этом
        exit(1);
    }
    else
    {
        int num_points;
        input_file >> num_points;
        std::cout << num_points;

        for (int i = 0; i < num_points; i++) {
            Point p;
            input_file >> p.x_ >> p.y_;
            points.push_back(p);
        }
        input_file.close();
    }

    triangulation(points);

    std::system("pdflatex visualisation.tex");

    return 0;
}
