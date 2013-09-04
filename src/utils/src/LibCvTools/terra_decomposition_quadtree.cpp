#include "terra_decomposition_quadtree.h"

TerraQuadtreeDecomposition::TerraQuadtreeDecomposition(const cv::Mat &_image, const cv::Size &_min_region_size,
                                                       TerraDecomClassifier::Ptr _classifier) :
    QuadtreeDecomposition(_image, _min_region_size, _classifier)
{
}

bool TerraQuadtreeDecomposition::iterate()
{
    TerraDecomClassifier *terra_classifier = dynamic_cast<TerraDecomClassifier*>(classifier_.get());

    if(quadtree_nodes_.size() == 0) {
        std::vector<cv::Rect> regions;
        std::vector<bool>     classifications;
        for(CVQtNodesList::iterator it = quadtree_roots_.begin() ; it != quadtree_roots_.end() ; it++)
            regions.push_back(*it);

        terra_classifier->classify(regions, classifications);

        for(int i = 0 ; i < quadtree_roots_.size() ; i++) {
            if(classifications[i] && !min_size_reached(quadtree_roots_[i]))
                split_and_activate(quadtree_roots_[i]);
            else {
                CVQt *node = &(quadtree_roots_[i]);
                cv_roi::TerraID p;
                p.id    = terra_classifier->get_id();
                p.prob  = terra_classifier->get_prob();
                classifications_.insert(ClassificationEntry(node, p));
                quadtree_leaves_.push_back(node);
            }

        }
    } else {
        process_active_nodes();
    }

    if(debug_size_.width != -1 && debug_size_.height != -1)
        render_debug();

    return quadtree_nodes_.size() != 0;
}

void TerraQuadtreeDecomposition::regions(std::vector<cv_roi::TerraROI> &regions)
{
    for(CVQtNodesListPtr::iterator it = quadtree_leaves_.begin() ; it != quadtree_leaves_.end() ; it++) {
        cv_roi::TerraROI tr;
        tr.roi.rect = *(*it);
        tr.id = classifications_[*it];
        regions.push_back(tr);
    }
}

void TerraQuadtreeDecomposition::process_active_nodes()
{
    TerraDecomClassifier *terra_classifier = dynamic_cast<TerraDecomClassifier*>(classifier_.get());

    CVQtNodesListPtr list = quadtree_nodes_;
    quadtree_nodes_.clear();

    for(CVQtNodesListPtr::iterator it = list.begin() ; it != list.end() ; it++){
        CVQt *node = *it;
        if(!min_size_reached(*node) && terra_classifier->classify(*node)) {
            split_and_activate(*node);
        } else {
            cv_roi::TerraID p;
            p.id    = terra_classifier->get_id();
            p.prob  = terra_classifier->get_prob();
            classifications_.insert(ClassificationEntry(node, p));
            quadtree_leaves_.push_back(node);
        }
    }
}

