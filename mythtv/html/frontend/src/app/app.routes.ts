import { Routes } from '@angular/router';
import { Status } from './status/status';
import { Remote } from './remote/remote';

export const routes: Routes = [
    { path: '', component: Status },
    { path: 'remote', component: Remote },
];
