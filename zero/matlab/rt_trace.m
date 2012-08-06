function varargout = rt_trace(varargin)
% RT_TRACE MATLAB code for rt_trace.fig
%      RT_TRACE, by itself, creates a new RT_TRACE or raises the existing
%      singleton*.
%
%      H = RT_TRACE returns the handle to a new RT_TRACE or the handle to
%      the existing singleton*.
%
%      RT_TRACE('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in RT_TRACE.M with the given input arguments.
%
%      RT_TRACE('Property','Value',...) creates a new RT_TRACE or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before rt_trace_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to rt_trace_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help rt_trace

% Last Modified by GUIDE v2.5 31-Jul-2012 22:30:41

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @rt_trace_OpeningFcn, ...
                   'gui_OutputFcn',  @rt_trace_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


function updateplot_fcn (obj, event)
% Check for new batch and plot
%disp ('Checking for new data..');
persistent lastid;
persistent last_n_traces;
persistent last_show_seconds;
persistent last_show_magnitudes;

if isempty(lastid) || isempty(last_n_traces) || isempty(last_show_seconds) ...
    || isempty(last_show_magnitudes)
  lastid = 0;
  last_n_traces = 0;
  last_show_seconds = 0;
  last_show_magnitudes = 0;
end

handles = obj.UserData;

try
  n_traces = sscanf (get(handles.ed_traces_to_show, 'String'), '%d');
catch me
  disp ('Invalid number of traces, setting to 5.')
  n_traces = 5;
end

mpath  = sprintf ('log/%s/', handles.buoy);
files = sprintf ('log/%s/*.DTT', handles.buoy);

a = dir(files);
l = length(a);

if get(handles.cb_seconds, 'Value')
  seconds_axis = true;
else
  seconds_axis = false;
end

if get(handles.cb_magnitude, 'Value')
  show_magnitudes = true;
else
  show_magnitudes = false;
end

% Sorting by id number
n = [];
for i=1:l
  n = [n sscanf(a(i).name, '%d.DTT')];
end

[n, i] = sort(n);

n_traces = min(n_traces, length(n));

if (length(n)) < 1
  disp ('No data.');
  return;
end

if (n(end) > lastid) || (seconds_axis ~= last_show_seconds) || ...
    (last_n_traces ~= n_traces) || (last_show_magnitudes ~= show_magnitudes)
  disp ('New data or change in preferences, updating plot..');
  set (handles.txt_status, 'String', 'Updating..');
  cd (mpath)
  [t, d] = readrangedtt (n(end-n_traces:end));
  if seconds_axis
    t = linspace (0, length(t)/250, length(t));
  end
  plotdtt (t, d, show_magnitudes, false, handles.tr_axes);
  cd ('../../');
  lastid = n(end);
  last_n_traces = n_traces;
  last_show_seconds = seconds_axis;
  last_show_magnitudes = show_magnitudes;
  set (handles.txt_status, 'String', '');
end

return;

% --- Executes just before rt_trace is made visible.
function rt_trace_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to rt_trace (see VARARGIN)

% Choose default command line output for rt_trace
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% Set to monitor on start
set(handles.tb_monitor, 'Value', 1);

% Set up buoy
handles.buoy = 'Four';

% Set up timer
handles.updateplot_t = timer ('ExecutionMode', 'fixedDelay', 'BusyMode', 'drop', 'Period', 5.0, 'TimerFcn', @updateplot_fcn, 'UserData', [handles], 'StartDelay', 2);
guidata (hObject, handles);
start (handles.updateplot_t);

% UIWAIT makes rt_trace wait for user response (see UIRESUME)
% uiwait(handles.figure1);




% --- Outputs from this function are returned to the command line.
function varargout = rt_trace_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



% --- Executes on button press in tb_monitor.
function tb_monitor_Callback(hObject, eventdata, handles)
% hObject    handle to tb_monitor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tb_monitor
if get(hObject, 'Value')
  start (handles.updateplot_t);
else
  stop (handles.updateplot_t);
end




% --- If Enable == 'on', executes on mouse press in 5 pixel border.
% --- Otherwise, executes on mouse press in 5 pixel border or over tb_monitor.
function tb_monitor_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to tb_monitor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
 


% --- Executes on button press in btn_close.
function btn_close_Callback(hObject, eventdata, handles)
% hObject    handle to btn_close (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close (handles.output)


% --- Executes during object deletion, before destroying properties.
function figure1_DeleteFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop (handles.updateplot_t);
delete (handles.updateplot_t);
clear all;


% --- Executes on button press in cb_1.
function cb_1_Callback(hObject, eventdata, handles)
% hObject    handle to cb_1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_1


% --- Executes on button press in cb_2.
function cb_2_Callback(hObject, eventdata, handles)
% hObject    handle to cb_2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_2


% --- Executes on button press in cb_3.
function cb_3_Callback(hObject, eventdata, handles)
% hObject    handle to cb_3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_3


% --- Executes on button press in cb_4.
function cb_4_Callback(hObject, eventdata, handles)
% hObject    handle to cb_4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_4


% --- Executes on button press in cb_5.
function cb_5_Callback(hObject, eventdata, handles)
% hObject    handle to cb_5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_5



function ed_traces_to_show_Callback(hObject, eventdata, handles)
% hObject    handle to ed_traces_to_show (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of ed_traces_to_show as text
%        str2double(get(hObject,'String')) returns contents of ed_traces_to_show as a double


% --- Executes during object creation, after setting all properties.
function ed_traces_to_show_CreateFcn(hObject, eventdata, handles)
% hObject    handle to ed_traces_to_show (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in cb_seconds.
function cb_seconds_Callback(hObject, eventdata, handles)
% hObject    handle to cb_seconds (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_seconds


% --- Executes on button press in cb_magnitude.
function cb_magnitude_Callback(hObject, eventdata, handles)
% hObject    handle to cb_magnitude (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cb_magnitude
