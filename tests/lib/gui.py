import enum
import asyncio
import inspect
import threading

from tkinter import *
from tkinter.ttk import Combobox, Scrollbar

import tkinter.font as TkFont

from api.arduino import ArduinoConnection
from api import APIClient
from api.exceptions import APIException

FONT_SIZE = 12

API_METHODS = {
    'Create Water Source': 'create_water_source',
    'Get Water Source': 'get_water_source',
    'Set Water Source State': 'set_water_source_state',
    'Set Water Source Active': 'set_water_source_active',
    'Remove Water Source': 'remove_water_source',
    'Get Water Source List': 'get_water_source_list',
    'Create Water Tank': 'create_water_tank',
    'Remove Water Tank': 'remove_water_tank',
    'Set Water Tank Minimum Volume': 'set_water_tank_minimum_volume',
    'Set Water Tank Max Volume': 'set_water_tank_max_volume',
    'Set Water Tank Zero Volume Pressure': 'set_water_tank_zero_volume_pressure',
    'Set Water Tank Volume Factor': 'set_water_tank_volume_factor',
    'Set Water Tank Pressure Factor': 'set_water_tank_pressure_factor',
    'Set Water Tank Pressure Changing Value': 'set_water_tank_pressure_changing_value',
    'Get Water Tank List': 'get_water_tank_list',
    'Get Water Tank': 'get_water_tank',
    'Fill Water Tank': 'fill_water_tank',
    'Set Water Tank Active': 'set_water_tank_active',
    'Set Operation Mode': 'set_operation_mode',
    'Get Operation Mode': 'get_operation_mode',
    'Save': 'save',
    'Reset': 'reset',
    'Create IO': 'create_io',
    'Set IO Value': 'set_io_value',
    'Get IO Value': 'get_io_value',
    'Clear IO': 'clear_io',
    'Set IO Source': 'set_io_source',
    'Set Clock Offset': 'set_clock_offset',
    'Get Millis': 'get_millis',
    'Get Free Memory': 'get_free_memory',
    'Reset Clock': 'reset_clock'
}


class Window(Tk):
    def __init__(self, loop, interval=1/120, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.title('Water Manager GUI')

        self._font = TkFont.Font(family='Arial', size=FONT_SIZE)
        self.option_add('*TCombobox*Listbox*Font', self._font)

        self.protocol("WM_DELETE_WINDOW", self.close)

        self._loop = loop

        self._arduino_connection = None
        self._api_client : APIClient = None
        self._event_loop = loop

        self._method_param_values = dict()

        self._read_responses_event = asyncio.Event()
        self._response_futures = asyncio.Queue()

        self._tasks = []
        self._tasks.append(loop.create_task(self._updater(interval)))
        self._tasks.append(loop.create_task(self._read_error_responses()))
        self._tasks.append(loop.create_task(self._read_responses()))

        # Widgets
        self._selected_method = None

        self._methods_frame = None
        self._method_frame = None

        self._main_frame = Frame(self)
        self._main_frame.grid(row=0, column=0, padx=20, pady=20, sticky=N)

        self._console_frame = Frame(self)
        self._console_frame.grid(row=0, column=1, padx=20, pady=20, rowspan=2, sticky=N+S)

        y_console_scroll = Scrollbar(self._console_frame)
        y_console_scroll.grid(row=0, column=1, stick=N+S)

        x_console_scroll = Scrollbar(self._console_frame, orient='horizontal')
        x_console_scroll.grid(row=1, column=0, stick=W+E)

        self._console = Listbox(self._console_frame, yscrollcommand=y_console_scroll.set,
                                bg='#fff', justify=LEFT, xscrollcommand=x_console_scroll.set,
                                borderwidth=2, relief='groove',
                                height=30, width=50, font=self._font)
        self._console.grid(row=0, column=0, sticky=N+S)

        y_console_scroll.config(command=self._console.yview) 
        x_console_scroll.config(command=self._console.xview) 

        comport_frame = Frame(self._main_frame)
        comport_frame.grid(row=0, column=0, pady=(0, 20))

        comport_title = Label(comport_frame, text='Select the comport that the water manager is connected to it', font=self._font)
        comport_title.grid(row=0, column=0, columnspan=2, pady=(0, 5))

        comport_label = Label(comport_frame, text='COM: ', font=self._font)
        comport_label.grid(row=1, column=0, sticky=W)

        comport_combobox = Combobox(comport_frame, values=ArduinoConnection.available_comports(), font=self._font)
        comport_combobox.grid(row=1, column=1, sticky=W+E)

        comport_combobox.bind('<<ComboboxSelected>>', self._on_select_comport)

    def close(self):
        for task in self._tasks:
            task.cancel()
        self._loop.stop()
        self.destroy()
    
    async def _updater(self, interval):
        while True:
            self.update()
            await asyncio.sleep(interval)

    def _on_select_comport(self, event):
        self._arduino_connection = ArduinoConnection(port=event.widget.get())
        self._api_client = APIClient(self._arduino_connection, self._event_loop)
        event.widget.configure(state='disabled')
        self._draw_method_selector()

        self._read_responses_event.set()

    def _on_select_method(self, event):
        method_name = API_METHODS[event.widget.get()]
        method = getattr(self._api_client, method_name)
        parameters = inspect.getfullargspec(method)[6].copy()

        if 'return' in parameters:
            del parameters['return']

        self._selected_method = method

        for widget in self._method_frame.winfo_children():
            widget.destroy()

        self._method_param_values = dict()

        i = 0
        for i, (param, type_) in enumerate(parameters.items()):
            label = Label(self._method_frame, text=f'{param.replace("_", " ").title()}: ', font=self._font)
            label.grid(row=i, column=0, sticky=W)

            entry = None
            var = StringVar()
            if type_ is str:
                entry = Entry(self._method_frame, font=self._font, textvariable=var)
            elif type_ is int:
                entry = Spinbox(self._method_frame, font=self._font, from_=0, to=float('inf'), increment=1, textvariable=var)
            elif type_ is float:
                entry = Spinbox(self._method_frame, font=self._font, from_=0, to=float('inf'), format='%.3f', increment=0.1, textvariable=var)
            elif type_ is bool:
                entry = Combobox(self._method_frame, font=self._font, textvariable=var, values=['TRUE', 'FALSE'])
            elif issubclass(type_, enum.Enum):
                entry = Combobox(self._method_frame, font=self._font, textvariable=var, values=[t.name for t in type_])

            self._method_param_values[param] = var, type_
            entry.grid(row=i, column=1, pady=10, sticky=W+E)

        submit_button = Button(self._method_frame, text='Submit', font=self._font, command=self._on_submit)
        submit_button.grid(row=i + 1, column=1, columnspan=2, pady=(20, 0), sticky=W+E)


    def _draw_method_selector(self):
        self._methods_frame = Frame(self._main_frame)
        self._methods_frame.grid(row=1, column=0, sticky=W+E)

        method_title = Label(self._methods_frame, text='Select a method, fill the paramerts to call it', font=self._font)
        method_title.grid(row=0, column=0, columnspan=2, sticky=W, pady=(0, 5))

        method_label = Label(self._methods_frame, anchor=W, text='Method: ', font=self._font)
        method_label.grid(row=1, column=0, sticky=W)

        method_combobox = Combobox(self._methods_frame, width=35, values=list(API_METHODS.keys()), font=self._font)
        method_combobox.grid(row=1, column=1, sticky=W+E)

        method_combobox.bind('<<ComboboxSelected>>', self._on_select_method)

        self._method_frame = Frame(self._main_frame)
        self._method_frame.grid(row=2, column=0, sticky=W+E, pady=(20, 0))
    
    def _on_submit(self):
        args = dict()
        for param, (var, type_) in self._method_param_values.items():
            if type_ in (str, int, float):
                args[param] = type_(var.get())
            elif type_ is bool:
                args[param] = var.get() == 'TRUE'
            elif issubclass(type_, enum.Enum):
                args[param] = getattr(type_, var.get())
            
            if type_ is str and not args[param]:
                del args[param]

        coroutine = self._selected_method(**args, return_exceptions=True)
        self._loop.create_task(self._response_futures.put(coroutine))

    async def _read_error_responses(self):
        while True:
            await self._read_responses_event.wait()

            response = await self._api_client.get_error_response()

            self._console.insert(END, repr(response))

    async def _read_responses(self):
        while True:
            await self._read_responses_event.wait()

            coroutine = await self._response_futures.get()
            response = await coroutine
            self._console.insert(END, repr(response))


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    window = Window(loop)
    loop.run_forever()
    loop.close()
