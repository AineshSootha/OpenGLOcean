#include "oceancalcs.hpp"

#define PI 3.14159265358979323846264338327950288

glm::vec2 calculateGaussian(GLfloat u1, GLfloat u2){
    GLfloat temp1 = 2 * PI * u2;
    GLfloat temp2 = sqrt(-2 * log(u1));
    return glm::vec2(temp2 * cos(temp1), temp2 * sin(temp1));
}


Ocean::Ocean(GLint _N, GLint _L, GLint _ws, GLint _A, int w /*= 256*/, int h /*= 256*/) : 
    N(_N),
    L(_L),
    windspeed(_ws),
    A(_A),
    rands1(w*h*3, 0.0f),
    rands2(w*h*3, 0.0f) {}

int Ocean::generateRands(int runCall, int w = 256, int h = 256){
    // ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;
    std::vector<GLfloat> reds1(w*h, 0.0f);
    std::vector<GLfloat> greens1(w*h, 0.0f);
    std::vector<GLfloat> reds2(w*h, 0.0f);
    std::vector<GLfloat> greens2(w*h, 0.0f);
    // std::vector<GLfloat> gaussian(w*h*3, 0.0f);
    for(int curr = 0; curr < w*h; curr++){
        reds1.at(curr) = rand() % (256);
        reds1.at(curr) /= 256.0f;
        greens1.at(curr) = rand() % (256);
        greens1.at(curr) /= 256.0f;
        reds2.at(curr) = rand() % (256);
        reds2.at(curr) /= 256.0f;
        greens2.at(curr) = rand() % (256);
        greens2.at(curr) /= 256.0f;
    }
    for(int curr = 0; curr < w*h*3; curr+=3){
        rands1.at(curr + 0) = reds1.at(curr/3); 
        rands1.at(curr + 1) = greens1.at(curr/3); 
        rands1.at(curr + 2) = 0.0f; 
        rands2.at(curr + 0) = reds2.at(curr/3); 
        rands2.at(curr + 1) = greens2.at(curr/3); 
        rands2.at(curr + 2) = 0.0f; 
    }

    //TESTING ONLY
    std::ofstream ofs("temp/test_gaussians" + std::to_string(1) + ".ppm", std::ios_base::out | std::ios_base::binary);
    std::ofstream ofs2("temp/test_gaussians" + std::to_string(1) + ".txt");
    ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
    for(int curr = 0; curr < w*h*3; curr+=3){
        // std::cout<<curr<<std::endl;
        ofs << (char) (rands1.at(curr + 0) * 256) << (char) (rands1.at(curr + 1) * 256) << (char) (rands1.at(curr + 2) * 256);
        ofs2 <<  (rands1.at(curr + 0)) << " " <<  (rands1.at(curr + 1)) << " " <<  (rands1.at(curr + 2)) << '\n';
    }
    ofs.close();
    ofs2.close();

    ofs.open("temp/test_gaussians" + std::to_string(2) + ".ppm", std::ios_base::out | std::ios_base::binary);
    ofs2.open("temp/test_gaussians" + std::to_string(2) + ".txt");
    ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
    for(int curr = 0; curr < w*h*3; curr+=3){
        // std::cout<<curr<<std::endl;
        ofs << (char) (rands2.at(curr + 0) * 256) << (char) (rands2.at(curr + 1) * 256) << (char) (rands2.at(curr + 2) * 256);
        ofs2 <<  (rands2.at(curr + 0)) << " " <<  (rands2.at(curr + 1)) << " " <<  (rands2.at(curr + 2)) << '\n';
    }
    ofs.close();
    ofs2.close();
    //****

    return 0; 
}


std::vector<GLfloat> Ocean::getGaussianRands(int id){
    switch(id){
        case 1:
            return rands1;
        case 2:
            return rands2;
        default:
            return std::vector<GLfloat>({-1.0f});
    }
    return std::vector<GLfloat>({-1.0f});
}

