//------------------------------------------------------------------------------
// Copyright 2018 H2O.ai
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//------------------------------------------------------------------------------
#include "frame/py_frame.h"
#include "python/_all.h"
#include "python/string.h"
#include "str/py_str.h"
#include <vector>
#include "models/py_ftrl.h"
#include "models/py_validator.h"
#include "models/utils.h"

namespace py {

PKArgs Ftrl::Type::args___init__(0, 2, 8, false, false,
                                 {"params", "labels", "alpha", "beta",
                                 "lambda1", "lambda2", "nbins", "nepochs",
                                 "interactions", "double_precision"},
                                 "__init__", nullptr);


void Ftrl::m__init__(PKArgs& args) {
  dtft = nullptr;
  dt::FtrlParams ftrl_params;

  bool defined_params           = !args[0].is_none_or_undefined();
  bool defined_labels           = !args[1].is_none_or_undefined();
  bool defined_alpha            = !args[2].is_none_or_undefined();
  bool defined_beta             = !args[3].is_none_or_undefined();
  bool defined_lambda1          = !args[4].is_none_or_undefined();
  bool defined_lambda2          = !args[5].is_none_or_undefined();
  bool defined_nbins            = !args[6].is_none_or_undefined();
  bool defined_nepochs          = !args[7].is_none_or_undefined();
  bool defined_interactions     = !args[8].is_none_or_undefined();
  bool defined_double_precision = !args[9].is_none_or_undefined();

  if (defined_params) {
    if (defined_alpha || defined_beta || defined_lambda1 || defined_lambda2 ||
        defined_nbins || defined_nepochs || defined_interactions ||
        defined_double_precision) {
      throw TypeError() << "You can either pass all the parameters with "
            << "`params` or any of the individual parameters with `alpha`, "
            << "`beta`, `lambda1`, `lambda2`, `nbins`, `nepochs`, "
            << "`interactions` or `double_precision` to Ftrl constructor, "
            << "but not both at the same time";
    }
    py::otuple py_params = args[0].to_otuple();
    py::oobj py_alpha = py_params.get_attr("alpha");
    py::oobj py_beta = py_params.get_attr("beta");
    py::oobj py_lambda1 = py_params.get_attr("lambda1");
    py::oobj py_lambda2 = py_params.get_attr("lambda2");
    py::oobj py_nbins = py_params.get_attr("nbins");
    py::oobj py_nepochs = py_params.get_attr("nepochs");
    py::oobj py_interactions = py_params.get_attr("interactions");
    py::oobj py_double_precision = py_params.get_attr("double_precision");

    ftrl_params.alpha = py_alpha.to_double();
    ftrl_params.beta = py_beta.to_double();
    ftrl_params.lambda1 = py_lambda1.to_double();
    ftrl_params.lambda2 = py_lambda2.to_double();
    ftrl_params.nbins = static_cast<uint64_t>(py_nbins.to_size_t());
    ftrl_params.nepochs = py_nepochs.to_size_t();
    ftrl_params.interactions = py_interactions.to_bool_strict();
    ftrl_params.double_precision = py_double_precision.to_bool_strict();

    py::Validator::check_positive<double>(ftrl_params.alpha, py_alpha);
    py::Validator::check_not_negative<double>(ftrl_params.beta, py_beta);
    py::Validator::check_not_negative<double>(ftrl_params.lambda1, py_lambda1);
    py::Validator::check_not_negative<double>(ftrl_params.lambda2, py_lambda2);
    py::Validator::check_not_negative<double>(ftrl_params.nbins, py_nbins);

  } else {

    if (defined_alpha) {
      ftrl_params.alpha = args[2].to_double();
      py::Validator::check_positive<double>(ftrl_params.alpha, args[2]);
    }

    if (defined_beta) {
      ftrl_params.beta = args[3].to_double();
      py::Validator::check_not_negative<double>(ftrl_params.beta, args[3]);
    }

    if (defined_lambda1) {
      ftrl_params.lambda1 = args[4].to_double();
      py::Validator::check_not_negative<double>(ftrl_params.lambda1, args[4]);
    }

    if (defined_lambda2) {
      ftrl_params.lambda2 = args[5].to_double();
      py::Validator::check_not_negative<double>(ftrl_params.lambda2, args[5]);
    }

    if (defined_nbins) {
      ftrl_params.nbins = static_cast<uint64_t>(args[6].to_size_t());
      py::Validator::check_positive<uint64_t>(ftrl_params.nbins, args[6]);
    }

    if (defined_nepochs) {
      ftrl_params.nepochs = args[7].to_size_t();
    }

    if (defined_interactions) {
      ftrl_params.interactions = args[8].to_bool_strict();
    }
    if (defined_double_precision) {
      ftrl_params.double_precision = args[9].to_bool_strict();
    }
  }

  if (defined_labels) {
    set_labels(args[1]);
  } else {
    py::olist py_list(0);
    set_labels(py::robj(py_list));
  }

  if (ftrl_params.double_precision) {
    dtft = new dt::Ftrl<double>(ftrl_params);
  } else {
    dtft = new dt::Ftrl<float>(ftrl_params);
  }
  reg_type = RegType::NONE;
}


//------------------------------------------------------------------------------
// fit()
//------------------------------------------------------------------------------

static PKArgs args_fit(2, 0, 0, false, false, {"X", "y"}, "fit",
R"(fit(self, X, y)
--

Train an FTRL model on a dataset.

Parameters
----------
X: Frame
    Frame of shape (nrows, ncols) to be trained on.

y: Frame
    Frame of shape (nrows, 1), i.e. the target column.
    This column must have a `bool` type.

Returns
-------
    None
)");


void Ftrl::fit(const PKArgs& args) {
  if (args[0].is_undefined()) {
    throw ValueError() << "Training frame parameter is missing";
  }

  if (args[1].is_undefined()) {
    throw ValueError() << "Target frame parameter is missing";
  }

  DataTable* dt_X = args[0].to_frame();
  DataTable* dt_y = args[1].to_frame();
  if (dt_X == nullptr || dt_y == nullptr) return;

  if (dt_X->ncols == 0) {
    throw ValueError() << "Training frame must have at least one column";
  }

  if (dt_X->nrows == 0) {
    throw ValueError() << "Training frame cannot be empty";
  }

  if (dt_y->ncols != 1) {
    throw ValueError() << "Target frame must have exactly one column";
  }

  if (dt_X->nrows != dt_y->nrows) {
    throw ValueError() << "Target column must have the same number of rows "
                       << "as the training frame";
  }

  dtft->dispatch_fit(dt_X, dt_y);
}


//------------------------------------------------------------------------------
// predict()
//------------------------------------------------------------------------------

static PKArgs args_predict(1, 0, 0, false, false, {"X"}, "predict",
R"(predict(self, X)
--

Make predictions for a dataset.

Parameters
----------
X: Frame
    Frame of shape (nrows, ncols) to make predictions for.
    It must have the same number of columns as the training frame.

Returns
-------
    A new frame of shape (nrows, 1) with the predicted probability
    for each row of frame X.
)");


oobj Ftrl::predict(const PKArgs& args) {
  if (args[0].is_undefined()) {
    throw ValueError() << "Frame to make predictions for is missing";
  }

  DataTable* dt_X = args[0].to_frame();
  if (dt_X == nullptr) return Py_None;

  if (!dtft->is_trained()) {
    throw ValueError() << "Cannot make any predictions, train or set "
                       << "the model first";
  }

  size_t ncols = dtft->get_dt_X_ncols();
  if (dt_X->ncols != ncols && ncols != 0) {
    throw ValueError() << "Can only predict on a frame that has " << ncols
                       << " column" << (ncols == 1? "" : "s")
                       << ", i.e. has the same number of features as "
                          "was used for model training";
  }


  DataTable* dt_p = dtft->predict(dt_X).release();
  py::oobj df_p = py::oobj::from_new_reference(
                         py::Frame::from_datatable(dt_p)
                  );

  return df_p;
}



void Ftrl::reset_feature_names() {
  if (feature_names != nullptr) {
    delete feature_names;
    feature_names = nullptr;
  }
}




//------------------------------------------------------------------------------
// reset()
//------------------------------------------------------------------------------

static PKArgs args_reset(0, 0, 0, false, false, {}, "reset",
R"(reset(self)
--

Reset FTRL model and feature importance information,
i.e. initialize model and importance frames with zeros.

Parameters
----------
    None

Returns
-------
    None
)");


void Ftrl::reset(const PKArgs&) {
  reg_type = RegType::NONE;
  dtft->reset();
}




//------------------------------------------------------------------------------
// .labels
//------------------------------------------------------------------------------

static GSArgs args_labels(
  "labels",
  R"(List of labels for multinomial regression.)");


oobj Ftrl::get_labels() const {
  return py::olist(labels);
}


void Ftrl::set_labels(robj py_labels) {
  py::olist labels_in = py_labels.to_pylist();
  size_t nlabels = labels_in.size();
  if (nlabels == 1) {
    throw ValueError() << "List of labels can not have one element";
  }
  // This ensures that we always have at least one classifier
  if (nlabels == 0) {
    labels_in.append(py::ostring("target"));
  }
  labels = labels_in;
}



//------------------------------------------------------------------------------
// .model
//------------------------------------------------------------------------------

static GSArgs args_model(
  "model",
R"(Tuple of model frames. Each frame has two columns, i.e. `z` and `n`,
and `nbins` rows, where `nbins` is a number of bins for the hashing
trick. Both column types are `T64`.)");


oobj Ftrl::get_model() const {
  if (dtft->is_trained()) {
    DataTable* dt_model = dtft->get_model();
    py::oobj df_model = py::oobj::from_new_reference(
                          py::Frame::from_datatable(dt_model)
                        );
    return df_model;
  } else {
    return py::None();
  }
}



void Ftrl::set_model(robj model) {
  // Reset model if it was assigned `None` in Python
  if (model.is_none()) {
    reg_type = RegType::NONE;
    dtft->reset();
    return;
  }

  DataTable* dt_model = model.to_frame();
  size_t ncols = dt_model->ncols;

  if (dt_model->nrows != dtft->get_nbins() || dt_model->ncols%2 != 0) {
    throw ValueError() << "Model frame must have " << dtft->get_nbins()
                       << " rows, and an even number of columns, "
                       << "whereas your frame has "
                       << dt_model->nrows << " row"
                       << (dt_model->nrows == 1? "": "s")
                       << " and "
                       << dt_model->ncols << " column"
                       << (dt_model->ncols == 1? "": "s");

  }

  bool double_precision = dtft->get_double_precision();
  SType stype = (double_precision)? SType::FLOAT32 : SType::FLOAT64;
  bool (*has_negatives)(const Column*) = (double_precision)? py::Validator::has_negatives<double> :
                                                       py::Validator::has_negatives<float>;

  for (size_t i = 0; i < ncols; ++i) {
    Column* col = dt_model->columns[i];
    SType c_stype = col->stype();
    if (col->stype() != stype) {
      throw ValueError() << "Column " << i << " in the model frame should "
                         << "have a type of " << stype << ", whereas your "
                         << "frame has the following column type: "
                         << c_stype;
    }

    if ((i % 2) && has_negatives(col)) {
      throw ValueError() << "Column " << i << " cannot have negative values";
    }

    dtft->set_model(dt_model);
  }
}


//------------------------------------------------------------------------------
// .fi
//------------------------------------------------------------------------------

static GSArgs args_fi(
  "feature_importances",
R"(One-column frame with the overall weight contributions calculated
feature-wise during training and predicting. It can be interpreted as
a feature importance information.)");


oobj Ftrl::get_fi() const {
  // If model was trained, return feature importance info.
  if (dtft->is_trained()) {
    DataTable* dt_fi = dtft->get_fi();
    py::oobj df_fi = py::oobj::from_new_reference(
                       py::Frame::from_datatable(dt_fi)
                     );
    return df_fi;
  } else {
    // If model was not trained, return `None`.
    return py::None();
  }
}


oobj Ftrl::get_fi_tuple() const {
  if (dtft->is_trained()) {
    py::otuple fi(1);
    DataTable* dt_fi = dtft->get_fi(false);
    py::oobj df_fi = py::oobj::from_new_reference(
      py::Frame::from_datatable(dt_fi)
    );
    fi.set(0, df_fi);
    return std::move(fi);
  } else {
    return py::None();
  }
}




//------------------------------------------------------------------------------
// .colname_hashes
//------------------------------------------------------------------------------

static GSArgs args_colname_hashes(
  "colname_hashes",
  "Column name hashes"
);


oobj Ftrl::get_colname_hashes() const {
  if (dtft->is_trained()) {
    size_t ncols = dtft->get_dt_X_ncols();
    py::otuple py_colname_hashes(ncols);
    std::vector<uint64_t> colname_hashes = dtft->get_colnames_hashes();
    for (size_t i = 0; i < ncols; ++i) {
      size_t h = static_cast<size_t>(colname_hashes[i]);
      py_colname_hashes.set(i, py::oint(h));
    }
    return std::move(py_colname_hashes);
  } else {
    return py::None();
  }
}




//------------------------------------------------------------------------------
// .alpha
//------------------------------------------------------------------------------

static GSArgs args_alpha(
  "alpha",
  "`alpha` in per-coordinate FTRL-Proximal algorithm");


oobj Ftrl::get_alpha() const {
  return py::ofloat(dtft->get_alpha());
}


void Ftrl::set_alpha(robj py_alpha) {
  double alpha = py_alpha.to_double();
  py::Validator::check_positive(alpha, py_alpha);
  dtft->set_alpha(alpha);
}




//------------------------------------------------------------------------------
// .beta
//------------------------------------------------------------------------------

static GSArgs args_beta(
  "beta",
  "`beta` in per-coordinate FTRL-Proximal algorithm");


oobj Ftrl::get_beta() const {
  return py::ofloat(dtft->get_beta());
}


void Ftrl::set_beta(robj py_beta) {
  double beta = py_beta.to_double();
  py::Validator::check_not_negative(beta, py_beta);
  dtft->set_beta(beta);
}




//------------------------------------------------------------------------------
// .lambda1
//------------------------------------------------------------------------------

static GSArgs args_lambda1(
  "lambda1",
  "L1 regularization parameter");


oobj Ftrl::get_lambda1() const {
  return py::ofloat(dtft->get_lambda1());
}


void Ftrl::set_lambda1(robj py_lambda1) {
  double lambda1 = py_lambda1.to_double();
  py::Validator::check_not_negative(lambda1, py_lambda1);
  dtft->set_lambda1(lambda1);
}




//------------------------------------------------------------------------------
// .lambda2
//------------------------------------------------------------------------------

static GSArgs args_lambda2(
  "lambda2",
  "L2 regularization parameter");


oobj Ftrl::get_lambda2() const {
  return py::ofloat(dtft->get_lambda2());
}


void Ftrl::set_lambda2(robj py_lambda2) {
  double lambda2 = py_lambda2.to_double();
  py::Validator::check_not_negative(lambda2, py_lambda2);
  dtft->set_lambda2(lambda2);
}




//------------------------------------------------------------------------------
// .nbins
//------------------------------------------------------------------------------

static GSArgs args_nbins(
  "nbins",
  "Number of bins to be used for the hashing trick");


oobj Ftrl::get_nbins() const {
  return py::oint(static_cast<size_t>(dtft->get_nbins()));
}


void Ftrl::set_nbins(robj py_nbins) {
  if (dtft->is_trained()) {
    throw ValueError() << "Cannot set `nbins` for a trained model, "
                       << "reset this model or create a new one";
  }

  uint64_t nbins = static_cast<uint64_t>(py_nbins.to_size_t());
  py::Validator::check_positive(nbins, py_nbins);
  dtft->set_nbins(nbins);
}




//------------------------------------------------------------------------------
// .nepochs
//------------------------------------------------------------------------------

static GSArgs args_nepochs(
  "nepochs",
  "Number of epochs to train a model");


oobj Ftrl::get_nepochs() const {
  return py::oint(dtft->get_nepochs());
}


void Ftrl::set_nepochs(robj py_nepochs) {
  size_t nepochs = py_nepochs.to_size_t();
  dtft->set_nepochs(nepochs);
}




//------------------------------------------------------------------------------
// .interactions
//------------------------------------------------------------------------------

static GSArgs args_interactions(
  "interactions",
  "Switch to enable second order feature interactions");


oobj Ftrl::get_interactions() const {
  return dtft->get_interactions()? True() : False();
}


void Ftrl::set_interactions(robj py_interactions) {
  bool interactions = py_interactions.to_bool_strict();
  dtft->set_interactions(interactions);
}


//------------------------------------------------------------------------------
// .double_precision
//------------------------------------------------------------------------------

static GSArgs args_double_precision(
  "double_precision",
  "Whether to use double precision arithmetic for modeling");


oobj Ftrl::get_double_precision() const {
  return dtft->get_double_precision()? True() : False();
}


void Ftrl::set_double_precision(robj py_double_precision) {
  bool double_precision = py_double_precision.to_bool_strict();
  dtft->set_interactions(double_precision);
}



//------------------------------------------------------------------------------
// .params
//------------------------------------------------------------------------------

static GSArgs args_params(
  "params",
  "FTRL model parameters");


oobj Ftrl::get_params_namedtuple() const {
  static onamedtupletype ntt(
    "FtrlParams",
    args_params.doc,
    {{args_alpha.name,        args_alpha.doc},
     {args_beta.name,         args_beta.doc},
     {args_lambda1.name,      args_lambda1.doc},
     {args_lambda2.name,      args_lambda2.doc},
     {args_nbins.name,        args_nbins.doc},
     {args_nepochs.name,      args_nepochs.doc},
     {args_interactions.name, args_interactions.doc},
     {args_double_precision.name, args_double_precision.doc}}
  );

  py::onamedtuple params(ntt);
  params.set(0, get_alpha());
  params.set(1, get_beta());
  params.set(2, get_lambda1());
  params.set(3, get_lambda2());
  params.set(4, get_nbins());
  params.set(5, get_nepochs());
  params.set(6, get_interactions());
  params.set(7, get_double_precision());
  return std::move(params);
}


void Ftrl::set_params_namedtuple(robj params) {
  set_alpha(params.get_attr("alpha"));
  set_beta(params.get_attr("beta"));
  set_lambda1(params.get_attr("lambda1"));
  set_lambda2(params.get_attr("lambda2"));
  set_nbins(params.get_attr("nbins"));
  set_nepochs(params.get_attr("nepochs"));
  set_interactions(params.get_attr("interactions"));
  set_double_precision(params.get_attr("double_precision"));
  // TODO: check that there are no unknown parameters
}


oobj Ftrl::get_params_tuple() const {
  py::otuple params(7);
  params.set(0, get_alpha());
  params.set(1, get_beta());
  params.set(2, get_lambda1());
  params.set(3, get_lambda2());
  params.set(4, get_nbins());
  params.set(5, get_nepochs());
  params.set(6, get_interactions());
  return std::move(params);
}


void Ftrl::set_params_tuple(robj params) {
  py::otuple params_tuple = params.to_otuple();
  size_t n_params = params_tuple.size();
  if (n_params != 8) {
    throw ValueError() << "Tuple of FTRL parameters should have 8 elements, "
                       << "got: " << n_params;
  }
  set_alpha(params_tuple[0]);
  set_beta(params_tuple[1]);
  set_lambda1(params_tuple[2]);
  set_lambda2(params_tuple[3]);
  set_nbins(params_tuple[4]);
  set_nepochs(params_tuple[5]);
  set_interactions(params_tuple[6]);
  set_double_precision(params_tuple[7]);
}


//------------------------------------------------------------------------------
// Pickling / unpickling methods.
//------------------------------------------------------------------------------

static PKArgs args___getstate__(
    0, 0, 0, false, false, {}, "__getstate__", nullptr);


oobj Ftrl::m__getstate__(const PKArgs&) {
  py::otuple pickle(4);
  py::oobj params = get_params_tuple();
  py::oobj model = get_model();
  py::oobj fi = get_fi_tuple();
  py::oobj py_reg_type = py::oint(static_cast<int32_t>(reg_type));

  pickle.set(0, params);
  pickle.set(1, model);
  pickle.set(2, fi);
  pickle.set(3, py_reg_type);
  return std::move(pickle);
}


static PKArgs args___setstate__(
    1, 0, 0, false, false, {"state"}, "__setstate__", nullptr);

void Ftrl::m__setstate__(const PKArgs& args) {
  m__dealloc__();
  py::otuple pickle = args[0].to_otuple();

  dt::FtrlParams ftrl_params;
  bool double_precision = (pickle[0].to_otuple())[7].to_bool_strict();
  if (double_precision) {
    dtft = new dt::Ftrl<double>(ftrl_params);
  } else {
    dtft = new dt::Ftrl<float>(ftrl_params);
  }

  // Set FTRL parameters
  set_params_tuple(pickle[0]);

  // Set model datatable.
  set_model(pickle[1]);

  // Set feature importance info
  if (pickle[2].is_frame()) {
    dtft->set_fi(pickle[2].to_frame()->copy());
  }


  // Set regression type
  reg_type = static_cast<RegType>(pickle[3].to_int32());

}



void Ftrl::m__dealloc__() {
  PyObject* py_labels = std::move(labels).release();
  Py_XDECREF(py_labels);
  if (dtft != nullptr) {
    delete dtft;
    dtft = nullptr;
  }
  reset_feature_names();
}



//------------------------------------------------------------------------------
// py::Ftrl::Type
//------------------------------------------------------------------------------

const char* Ftrl::Type::classname() {
  return "datatable.models.Ftrl";
}


// TODO: use the above doc strings here.
const char* Ftrl::Type::classdoc() {
  return R"(Follow the Regularized Leader (FTRL) model with hashing trick.

See this reference for more details:
https://www.eecs.tufts.edu/~dsculley/papers/ad-click-prediction.pdf

Parameters
----------
alpha : float
    `alpha` in per-coordinate learning rate formula.
beta : float
    `beta` in per-coordinate learning rate formula.
lambda1 : float
    L1 regularization parameter.
lambda2 : float
    L2 regularization parameter.
nbins : int
    Number of bins to be used after the hashing trick.
nepochs : int
    Number of epochs to train for.
interactions : bool
    Switch to enable second order feature interactions.
double_precision : bool
    Whether to use double precision arithmetic or not.
)";
}


void Ftrl::Type::init_methods_and_getsets(Methods& mm, GetSetters& gs)
{
  ADD_GETSET(gs, &Ftrl::get_labels, &Ftrl::set_labels, args_labels);
  ADD_GETSET(gs, &Ftrl::get_model, &Ftrl::set_model, args_model);
  ADD_GETTER(gs, &Ftrl::get_fi, args_fi);
  ADD_GETSET(gs, &Ftrl::get_params_namedtuple, &Ftrl::set_params_namedtuple,
             args_params);
  ADD_GETTER(gs, &Ftrl::get_colname_hashes, args_colname_hashes);
  ADD_GETSET(gs, &Ftrl::get_alpha, &Ftrl::set_alpha, args_alpha);
  ADD_GETSET(gs, &Ftrl::get_beta, &Ftrl::set_beta, args_beta);
  ADD_GETSET(gs, &Ftrl::get_lambda1, &Ftrl::set_lambda1, args_lambda1);
  ADD_GETSET(gs, &Ftrl::get_lambda2, &Ftrl::set_lambda2, args_lambda2);
  ADD_GETSET(gs, &Ftrl::get_nbins, &Ftrl::set_nbins, args_nbins);
  ADD_GETSET(gs, &Ftrl::get_nepochs, &Ftrl::set_nepochs, args_nepochs);
  ADD_GETSET(gs, &Ftrl::get_interactions, &Ftrl::set_interactions,
             args_interactions);

  ADD_METHOD(mm, &Ftrl::m__getstate__, args___getstate__);
  ADD_METHOD(mm, &Ftrl::m__setstate__, args___setstate__);
  ADD_METHOD(mm, &Ftrl::fit, args_fit);
  ADD_METHOD(mm, &Ftrl::predict, args_predict);
  ADD_METHOD(mm, &Ftrl::reset, args_reset);
}


} // namespace py
