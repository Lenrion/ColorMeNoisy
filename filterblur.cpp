#include "canvas2d.h"
#include "filterutils.h"
#include <iostream>

void Canvas2D::filterBlur(int blurRadius) {
    // if blur radius is 0, do nothing
    if (blurRadius != 0) {
        // calculate kernel vals
        int sideLen = 2 * blurRadius + 1;
        std::vector<float> kernel(sideLen, 0.0); // init vector
        float sum = 0.0;
        for (int i = 0; i < sideLen; i++) {
            // get row, col, distance from center of current thing in array
            float dist = abs(blurRadius - i);

            // calculate what should be there
            kernel[i] = 1.0 - (dist / blurRadius);

            // attempt at gaussian (not quite the same as the TA demo?)

            // float sigma = blurRadius / 3.0;
            // // float e = exp(1.0);
            // float power_term = -(pow(dist, 2) / (2.0 * pow(sigma, 2)));
            // float gaussian = (1/(sqrt(2 * M_PI * pow(sigma, 2)))) * pow(M_E, power_term);
            // kernel[i] = 1.0 - gaussian;

            sum += kernel[i];
        }
        for (int j = 0; j < kernel.size(); j++) {
            kernel[j] = kernel[j] / sum; // normalize
        }
        // copy image to convolve over
        std::vector<RGBAf> new_data(m_data.size(), RGBAf{0.0,0.0,0.0,0.0});
        for (int i = 0; i < m_data.size(); i++) {
            new_data[i] = RGBAf::fromRGBA(m_data[i]);
        }
        FilterUtils::convolveHorizontal(new_data, m_width, m_height, kernel);
        FilterUtils::convolveVertical(new_data, m_width, m_height, kernel);

        // apply to m_data
        for (int i = 0; i < m_data.size(); i++) {
            m_data[i].r = FilterUtils::clamp(new_data[i].r);
            m_data[i].g = FilterUtils::clamp(new_data[i].g);
            m_data[i].b = FilterUtils::clamp(new_data[i].b);

        }
    }

}
