#ifndef CAFFE_CONV_LSTM_LAYER_HPP
#define CAFFE_CONV_LSTM_LAYER_HPP

#include <string>
#include <vector>

#include "caffe/layers/recurrent_layer.hpp"
#include "caffe/layers/conv_lstm_helper.hpp"

namespace caffe{

  template <typename Dtype>
  class ConvLSTMLayer : public RecurrentLayer<Dtype> {
    public:
      explicit ConvLSTMLayer( const LayerParameter & param )
              : RecurrentLayer<Dtype>( param ) { }

      virtual inline const char * type() const {
        return "ConvLSTM";
      }

      virtual void LayerSetUp( const std::vector<Blob<Dtype>*> & bottom, const std::vector<Blob<Dtype>*> & top );
      virtual void Reshape(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);

      virtual inline int ExactNumTopBlobs() const {
        int num_tops = 1;

        num_tops += this->layer_param_.lstm_debug_param().expose_size();
            
        if( this->layer_param_.recurrent_param().expose_hidden() ){
          std::vector<std::string> outputs;
          this->RecurrentOutputBlobNames( &outputs );
          num_tops += outputs.size();
        }
        return num_tops;
      }

    protected:
      virtual void FillUnrolledNet( NetParameter * param ) const;
      virtual void RecurrentInputBlobNames( std::vector<std::string> * names ) const;
      virtual void RecurrentOutputBlobNames( std::vector<std::string> * names ) const;
      virtual void RecurrentInputShapes( std::vector<BlobShape> * shapes ) const;

      virtual void OutputBlobNames( std::vector<std::string> * names ) const;
      
    private:
      std::vector<int> in_shape_;
      int num_output_channels_ = 0;
  };

  template <typename Dtype>
  class ConvLSTMUnitLayer : public Layer<Dtype> {
    public:
      explicit ConvLSTMUnitLayer(const LayerParameter& param)
          : Layer<Dtype>(param) {}
      virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
          const vector<Blob<Dtype>*>& top);

      virtual inline const char* type() const { return "ConvLSTMUnit"; }
      virtual inline int ExactNumBottomBlobs() const { return 3; }
      virtual inline int ExactNumTopBlobs() const { return 2; }

      virtual inline bool AllowForceBackward(const int bottom_index) const {
        return bottom_index != 2;
      }

     protected:
      virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
          const vector<Blob<Dtype>*>& top);
      virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,
          const vector<Blob<Dtype>*>& top);

      virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
          const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
      virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,
          const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);

      int hidden_dim_;
      Blob<Dtype> X_acts_;
  };

}

#endif