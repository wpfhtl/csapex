#ifndef CMP_CORE_H
#define CMP_CORE_H
/// COMPONENT
#include "cmp_extractor_extended.h"
#include "cmp_randomforest_extended.h"
#include "roi.hpp"
/// SYSTEM
#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <utils/LibCvTools/grid.hpp>
#include <utils/LibCvTools/terra_decomposition_quadtree.h>
#include <utils/LibCvTools/grid_compute.hpp>

namespace YAML {
class Emitter;
}


class CMPCore
{
public:
    typedef boost::shared_ptr<CMPCore>                          Ptr;

    CMPCore();
    void        setWorkPath(const std::string &work_path);

    ///         IMAGE
    bool        loadImage(const std::string image_path);
    cv::Mat     getImage() const;
    ///         CLASSIFIER
    void        reload();
    std::string forestPath();

    ///     COMPUTATION
    void    compute();
    void    computeGrid();
    void    computeQuadtree();
    bool    hasComputedModel();

    ///     GET VISUALIZABLE RESULTS
    void    getGrid(std::vector<cv_roi::TerraROI> &cells);
    void    getQuad(std::vector<cv_roi::TerraROI> &regions);

    ///     PARAMETERS
    void    setExtractorParameters(cv_extraction::ExtractorParams &params);
    void    setRandomForestParams(const CMPForestParams &params);
    void    setKeyPointParameters(const cv_extraction::KeypointParams &params);
    void    write(YAML::Emitter &emitter) const;

    ///     PARAMS
    void    setGridParameters(const CMPGridParams &params);
    void    setQuadParameters(const CMPQuadParams &params);

    ///     TRAINING PREPARATION
    void    setRois(const std::vector<cv_roi::TerraROI> &rois);
    void    addClass(int classID);
    void    removeClass(int classID);
    void    getClasses(std::vector<int> &classes);


private:
    typedef CMPCVExtractorExt::KeyPoints                      KeyPoints;
    typedef boost::shared_ptr<cv_extraction::ExtractorParams> Params;

    cv::Mat                                 raw_image_;
    CMPCVExtractorExt::Ptr                  cv_extractor_;
    CMPPatternExtractorExt::Ptr             pt_extractor_;
    CMPRandomForestExt::Ptr                 random_;

    cv_extraction::KeypointParams           keypoint_params_;
    Params                                  ex_params_;

    TerraQuadtreeDecomposition::Ptr         quad_decom_;
    CMPQuadParams                           quad_params_;

    boost::shared_ptr<cv_grid::GridTerra>   grid_;
    CMPGridParams                           grid_params_;

    std::string                     work_path_;
    std::string                     file_extraction_;
    std::string                     file_forest_;
    std::vector<cv_roi::TerraROI>   rois_;
    std::vector<int>                classIDs_;


    void extract();
    void train();
};

#endif // CMP_CORE_H
