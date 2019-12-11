// ----------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License
// ----------------------------------------------------------------------
#pragma once

#include "Components/PipelineExecutionEstimatorImpl.h"
#include "Components/DocumentStatisticsEstimator.h"
#include "../Traits.h"

#include <cmath>

namespace Microsoft {
namespace Featurizer {
namespace Featurizers {

/////////////////////////////////////////////////////////////////////////
///  \struct        TFIDFStruct
///  \brief         Struct to hold return value of tfidf vectorizer,
///                 consist of <dictid, tfidf value>
///
struct TFIDFStruct {
    std::uint32_t const DictionaryId;                              // dict id
    std::float_t const TfidfValue;                                 // tfidf value

    TFIDFStruct(std::uint32_t dictionaryId, std::float_t tfidfValue);
    //FEATURIZER_MOVE_CONSTRUCTOR_ONLY(TFIDFStruct);
    bool operator==(TFIDFStruct const &other) const;
};

/////////////////////////////////////////////////////////////////////////
///  \class         TfidfVectorizerTransformer
///  \brief         Returns a unique TFIDFStruct for each input.
///
class TfidfVectorizerTransformer : public StandardTransformer<std::string, TFIDFStruct> {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using BaseType                           = StandardTransformer<std::string, TFIDFStruct>;
    using IndexMap                           = std::unordered_map<std::string, std::uint32_t>;
    using FrequencyMap                       = IndexMap;
    using IterRangeType                      = std::tuple<std::string::const_iterator, std::string::const_iterator>;
    using MapWithIterRange                   = std::map<IterRangeType, std::uint32_t, Components::IterRangeComp>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    IndexMap const                           Labels;
    FrequencyMap const                       DocumentFreq;
    std::uint32_t const                      TotalNumsDocuments;
    //Binary: If True, all non zero counts are set to 1. This is useful for discrete probabilistic models that model binary events rather than integer counts.
    bool const                               Binary;
    std::string const                        Norm;
    bool const                               UseIdf;
    bool const                               SmoothIdf;
    bool const                               SublinearTf;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    TfidfVectorizerTransformer(
        IndexMap labels, 
        FrequencyMap docufreq, 
        std::uint32_t totalnumdocus, 
        bool binary, 
        std::string norm, 
        bool use_idf, 
        bool smooth_idf, 
        bool sublinear_tf
    );
    TfidfVectorizerTransformer(Archive &ar);

    ~TfidfVectorizerTransformer(void) override = default;

    void save(Archive &ar) const override;

    bool operator==(TfidfVectorizerTransformer const &other) const;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------

    // MSVC has problems when the definition and declaration are separated
    void execute_impl(typename BaseType::InputType const &input, typename BaseType::CallbackFunction const &callback) override {
        //termfrequency for specific document
        MapWithIterRange documentTermFrequency;

        //todo: will use vector<functor> after string header file is done
        Components::split_temp( 
            input, 
            [&documentTermFrequency] (std::string::const_iterator & iterStart, std::string::const_iterator & iterEnd) {
                MapWithIterRange::iterator docuTermFreqIter(documentTermFrequency.find(std::make_tuple(iterStart, iterEnd)));
                if (docuTermFreqIter != documentTermFrequency.end()) {
                    ++docuTermFreqIter->second;
                } else {
                    documentTermFrequency.insert(std::make_pair(std::make_tuple(iterStart, iterEnd), 1));
                }
            }
        );

        std::float_t normVal = 0.0f;
        std::vector<std::tuple<std::uint32_t, std::float_t>> results;
        for (auto const & pair : documentTermFrequency) {
            std::string const word(std::string(std::get<0>(pair.first), std::get<1>(pair.first)));

            IndexMap::const_iterator const      labelIter(Labels.find(word));

            if (labelIter != Labels.end()) {
                
                std::float_t tf;
                std::float_t idf;

                //calculate tf
                if (Binary) {
                    tf = 1.0f;
                } else if (!SublinearTf) {
                    tf = static_cast<std::float_t>(pair.second);
                } else {
                    tf = 1.0f + static_cast<std::float_t>(log(pair.second));
                }

                //calculate idf;
                if (!UseIdf) {
                    idf = 1.0f;
                } else if (SmoothIdf) {
                    idf = 1.0f + log((1 + TotalNumsDocuments) / (1.0f + static_cast<std::float_t>(DocumentFreq.at(word))));
                } else {
                    idf = 1.0f + log((1 + TotalNumsDocuments) / (static_cast<std::float_t>(DocumentFreq.at(word))));
                }

                //calculate tfidf
                std::float_t tfidf = tf * idf;

                //calculate normVal
                if(Norm == "l1") {
                    normVal += abs(tfidf);
                } else if (Norm == "l2") {
                    normVal += tfidf * tfidf;
                }

                //temperarily put output in a vector for future normalization
                results.emplace_back(std::make_tuple(labelIter->second, tfidf));
            }
        }
        //normVal will never be 0 as long as results is not empty
        if (normVal == 0.0f)
            throw std::runtime_error("This happens when the input document is empty");

        // l2-norm calibration
        if (Norm == "l2")
            normVal = sqrt(normVal);

        for (auto & result : results) {
            callback(TFIDFStruct(std::get<0>(result), std::get<1>(result) / normVal));
        }    
    }                                                                                  

};

namespace Details {

/////////////////////////////////////////////////////////////////////////
///  \class         TfidfVectorizerEstimatorImpl
///  \brief         Estimator that uses the output of the 
///                 InverseDocumentFrequencyEstimator to provide useful
///                 information which helps calculation of TfidfVectorizer
///
template <size_t MaxNumTrainingItemsV=std::numeric_limits<size_t>::max()>
class TfidfVectorizerEstimatorImpl : public TransformerEstimator<std::string, TFIDFStruct> {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using BaseType                          = TransformerEstimator<std::string, TFIDFStruct>;
    using TransformerType                   = TfidfVectorizerTransformer;
    using IndexMap                          = TfidfVectorizerTransformer::IndexMap;
    using FrequencyMap                      = TfidfVectorizerTransformer::FrequencyMap;
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    TfidfVectorizerEstimatorImpl(
        AnnotationMapsPtr pAllColumnAnnotations, 
        size_t colIndex, 
        bool binary, 
        std::string norm, 
        bool use_idf, 
        bool smooth_idf, 
        bool sublinear_tf
    );
    ~TfidfVectorizerEstimatorImpl(void) override = default;

    FEATURIZER_MOVE_CONSTRUCTOR_ONLY(TfidfVectorizerEstimatorImpl);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data
    // |
    // ----------------------------------------------------------------------
    size_t const                             _colIndex;
    
    bool const                               _binary;

    std::string const                        _norm;
    bool const                               _use_idf;
    bool const                               _smooth_idf;
    bool const                               _sublinear_tf;

    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    bool begin_training_impl(void) override;

    // MSVC has problems when the declaration and definition are separated
    FitResult fit_impl(typename BaseType::InputType const *, size_t) override {
        throw std::runtime_error("This should not be called");
    }

    void complete_training_impl(void) override;

    // MSVC has problems when the declaration and definition are separated
    typename BaseType::TransformerUniquePtr create_transformer_impl(void) override {
       
        // ----------------------------------------------------------------------
        using DocumentStatisticsAnnotationData          = Components::DocumentStatisticsAnnotationData;
        using DocumentStatisticsEstimator               = Components::DocumentStatisticsEstimator<MaxNumTrainingItemsV>;

        // ----------------------------------------------------------------------

        DocumentStatisticsAnnotationData const &        data(DocumentStatisticsEstimator::get_annotation_data(BaseType::get_column_annotations(), _colIndex, Components::DocumentStatisticsEstimatorName));

        
        typename DocumentStatisticsAnnotationData::FrequencyAndIndexMap const & 
                                                        termFrequencyAndIndex(data.TermFrequencyAndIndex);
        std::uint32_t const                             totalNumDocus(data.TotalNumDocuments);

        FrequencyMap termFrequency;
        IndexMap termIndex;
        for (auto const & termFrequencyAndIndexPair : termFrequencyAndIndex) {
            //termFrequency and termIndex share the exactly same keys
            termFrequency.insert(std::make_pair(termFrequencyAndIndexPair.first, termFrequencyAndIndexPair.second.TermFrequency));
            termIndex.insert(std::make_pair(termFrequencyAndIndexPair.first, termFrequencyAndIndexPair.second.Index));
        }

        return std::make_unique<TfidfVectorizerTransformer>(std::move(termIndex), 
                                                            std::move(termFrequency),
                                                            std::move(totalNumDocus), 
                                                            std::move(_binary), 
                                                            std::move(_norm), 
                                                            std::move(_use_idf), 
                                                            std::move(_smooth_idf), 
                                                            std::move(_sublinear_tf)
                                                            );
    }
};

} // namespace Details

/////////////////////////////////////////////////////////////////////////
///  \class         TfidfVectorizerEstimator
///  \brief         Creates a `TfidfVectorizerTransformer` object.
///
template <size_t MaxNumTrainingItemsV=std::numeric_limits<size_t>::max()>
class TfidfVectorizerEstimator :
    public Components::PipelineExecutionEstimatorImpl<
        Components::DocumentStatisticsEstimator<MaxNumTrainingItemsV>,
        Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>
    > {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using BaseType =
        Components::PipelineExecutionEstimatorImpl<
            Components::DocumentStatisticsEstimator<MaxNumTrainingItemsV>,
            Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>
        >;

    using IndexMap                      = TfidfVectorizerTransformer::IndexMap;
    using StringDecorator               = Components::Details::DocumentStatisticsTrainingOnlyPolicy::StringDecorator;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    TfidfVectorizerEstimator(
        AnnotationMapsPtr pAllColumnAnnotations, 
        size_t colIndex, 
        bool binary = false,
        std::string norm = "l2", 
        bool use_idf = true, 
        bool smooth_idf = true, 
        bool sublinear_tf = false,
        std::float_t minDf = 0.0f, 
        std::float_t maxDf = 1.0f,
        StringDecorator decorator = StringDecorator(), 
        nonstd::optional<IndexMap> vocabulary = nonstd::optional<IndexMap>(), 
        nonstd::optional<std::uint32_t> maxFeatures = nonstd::optional<std::uint32_t>()
    );
    ~TfidfVectorizerEstimator(void) override = default;

    FEATURIZER_MOVE_CONSTRUCTOR_ONLY(TfidfVectorizerEstimator);
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// |
// |  TFIDFStruct
// |
// ----------------------------------------------------------------------
TFIDFStruct::TFIDFStruct(std::uint32_t dictionaryId, std::float_t tfidfValue) :
    DictionaryId(std::move(dictionaryId)),
    TfidfValue(std::move(tfidfValue)) {
}
bool TFIDFStruct::operator==(TFIDFStruct const &other) const {
    return (DictionaryId == other.DictionaryId) && (abs(TfidfValue - other.TfidfValue) < 0.000001f);
}

// ----------------------------------------------------------------------
// |
// |  TfidfVectorizerTransformer
// |
// ----------------------------------------------------------------------
TfidfVectorizerTransformer::TfidfVectorizerTransformer(IndexMap labels, IndexMap docufreq, std::uint32_t totalnumdocus, 
                                                       bool binary, std::string norm, bool use_idf, bool smooth_idf, bool sublinear_tf) :
    Labels(
        std::move(
            [&labels](void) ->  IndexMap & {
                if (labels.size() == 0) {
                    throw std::invalid_argument("Index map is empty!");
                }
                return labels;
            }()
        )
    ),
    DocumentFreq(
        std::move(
            [&docufreq](void) ->  IndexMap & {
                if (docufreq.size() == 0) {
                    throw std::invalid_argument("DocumentFrequency map is empty!");
                }
                return docufreq;
            }()
        )
    ),
    TotalNumsDocuments(std::move(totalnumdocus)),
    Binary(std::move(binary)), 
    Norm(std::move(norm)),
    UseIdf(std::move(use_idf)),
    SmoothIdf(std::move(smooth_idf)),
    SublinearTf(std::move(sublinear_tf)) {
}

TfidfVectorizerTransformer::TfidfVectorizerTransformer(Archive &ar) :
    // TODO: Labels(Traits<decltype(Labels)>::deserialize(ar)),
    // TODO: Documentfreq(Traits<decltype(Documentfreq)>::deserialize(ar)),
    TotalNumsDocuments(Traits<decltype(TotalNumsDocuments)>::deserialize(ar)),
    Binary(Traits<decltype(Binary)>::deserialize(ar)),
    Norm(Traits<decltype(Norm)>::deserialize(ar)), 
    UseIdf(Traits<decltype(UseIdf)>::deserialize(ar)), 
    SmoothIdf(Traits<decltype(SmoothIdf)>::deserialize(ar)), 
    SublinearTf(Traits<decltype(SublinearTf)>::deserialize(ar)) {
}

void TfidfVectorizerTransformer::save(Archive &ar) const /*override*/ {
    // TODO: Traits<decltype(Labels)>::serialize(ar, Labels);
    // TODO: Traits<decltype(Documentfreq)>::serialize(ar, Documentfreq);
    Traits<decltype(TotalNumsDocuments)>::serialize(ar, TotalNumsDocuments);
    Traits<decltype(Binary)>::serialize(ar, Binary);
    Traits<decltype(Norm)>::serialize(ar, Norm);
    Traits<decltype(UseIdf)>::serialize(ar, UseIdf);
    Traits<decltype(SmoothIdf)>::serialize(ar, SmoothIdf);
    Traits<decltype(SublinearTf)>::serialize(ar, SublinearTf);
}

bool TfidfVectorizerTransformer::operator==(TfidfVectorizerTransformer const &other) const {
    return Labels == other.Labels
        && DocumentFreq == other.DocumentFreq
        && TotalNumsDocuments == other.TotalNumsDocuments
        && Binary == other.Binary
        && Norm == other.Norm
        && UseIdf == other.UseIdf
        && SmoothIdf == other.SmoothIdf
        && SublinearTf == other.SublinearTf;
}

// ----------------------------------------------------------------------
// |
// |  TfidfVectorizerEstimator
// |
// ----------------------------------------------------------------------

template <size_t MaxNumTrainingItemsV>
TfidfVectorizerEstimator<MaxNumTrainingItemsV>::TfidfVectorizerEstimator(
    AnnotationMapsPtr pAllColumnAnnotations, 
    size_t colIndex, 
    bool binary,
    std::string norm, 
    bool use_idf, 
    bool smooth_idf, 
    bool sublinear_tf,
    std::float_t minDf, 
    std::float_t maxDf,
    StringDecorator decorator, 
    nonstd::optional<IndexMap> vocabulary, 
    nonstd::optional<std::uint32_t> maxFeatures
) :
    BaseType(
        "TfidfVectorizerEstimator",
        pAllColumnAnnotations,
        [pAllColumnAnnotations, colIndex, &decorator, &vocabulary, &maxFeatures, &minDf, &maxDf](void) { 
            return Components::DocumentStatisticsEstimator<MaxNumTrainingItemsV>(
                std::move(pAllColumnAnnotations), 
                std::move(colIndex),
                std::move(decorator),
                std::move(vocabulary),
                std::move(maxFeatures),
                std::move(minDf),
                std::move(maxDf)
            ); 
        },
        [pAllColumnAnnotations, colIndex, &binary, &norm, &use_idf, &smooth_idf, &sublinear_tf](void) { 
            return Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>(
                std::move(pAllColumnAnnotations), 
                std::move(colIndex), 
                std::move(binary),
                std::move(norm),
                std::move(use_idf),
                std::move(smooth_idf),
                std::move(sublinear_tf)
            ); 
        }
    ) {
}

// ----------------------------------------------------------------------
// |
// |  Details::TfidfVectorizerEstimatorImpl
// |
// ----------------------------------------------------------------------
template <size_t MaxNumTrainingItemsV>
Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>::TfidfVectorizerEstimatorImpl(
    AnnotationMapsPtr pAllColumnAnnotations, 
    size_t colIndex, 
    bool binary, 
    std::string norm, 
    bool use_idf, 
    bool smooth_idf, 
    bool sublinear_tf
) :
    BaseType("TfidfVectorizerEstimatorImpl", std::move(pAllColumnAnnotations)),
    _colIndex(
        std::move(
            [this, &colIndex](void) -> size_t & {
                if(colIndex >= this->get_column_annotations().size())
                    throw std::invalid_argument("colIndex");

                return colIndex;
            }()
        )
    ),
    _binary(std::move(binary)),
    _norm(
        std::move(
            [&norm](void) -> std::string & {
                if(norm != "l1" && norm != "l2")
                    throw std::invalid_argument("norm");

                return norm;
            }()
        )
    ),
    _use_idf(std::move(use_idf)),
    _smooth_idf(std::move(smooth_idf)),
    _sublinear_tf(std::move(sublinear_tf)) {
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
template <size_t MaxNumTrainingItemsV>
bool Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>::begin_training_impl(void) /*override*/ {
    return false;
}

template <size_t MaxNumTrainingItemsV>
void Details::TfidfVectorizerEstimatorImpl<MaxNumTrainingItemsV>::complete_training_impl(void) /*override*/ {
}

} // namespace Featurizers
} // namespace Featurizer
} // namespace Microsoft
